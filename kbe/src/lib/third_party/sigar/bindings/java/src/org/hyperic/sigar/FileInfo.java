/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.hyperic.sigar;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

public class FileInfo extends FileAttrs implements java.io.Serializable {

    private static final long serialVersionUID = 02242007L;

    private static final SimpleDateFormat DATE_FORMAT =
        new SimpleDateFormat("MMM dd HH:mm");

    String name;
    private transient Sigar sigar;
    private boolean dirStatEnabled = false;
    private DirStat stat = null;
    private boolean lstat;
    private FileInfo oldInfo = null;

    /**
     * No file type determined.
     */
    public static final int TYPE_NOFILE  = 0;
    /**
     * A regular file.
     */
    public static final int TYPE_REG     = 1;
    /**
     * A directory.
     */
    public static final int TYPE_DIR     = 2;
    /**
     * A character device.
     */
    public static final int TYPE_CHR     = 3;
    /**
     * A block device.
     */
    public static final int TYPE_BLK     = 4;
    /**
     * A FIFO / pipe.
     */
    public static final int TYPE_PIPE    = 5;
    /**
     * A symbolic link.
     */
    public static final int TYPE_LNK     = 6;
    /**
     * A [unix domain] socket.
     */
    public static final int TYPE_SOCK    = 7;
    /**
     * A file of unknown type.
     */
    public static final int TYPE_UNKFILE = 8;

    /**
     * Readable by user.
     */
    public static final int MODE_UREAD    =  0x0400;
    /**
     * Writable by user.
     */
    public static final int MODE_UWRITE   =  0x0200;
    /**
     * Executable by user.
     */
    public static final int MODE_UEXECUTE =  0x0100;

    /**
     * Readable by group.
     */
    public static final int MODE_GREAD    =  0x0040;
    /**
     * Writable by group.
     */
    public static final int MODE_GWRITE   =  0x0020;
    /**
     * Executable by group.
     */
    public static final int MODE_GEXECUTE =  0x0010;

    /**
     * Readable by others.
     */
    public static final int MODE_WREAD    =  0x0004;
    /**
     * Writable by others.
     */
    public static final int MODE_WWRITE   =  0x0002;
    /**
     * Executable by others.
     */
    public static final int MODE_WEXECUTE =  0x0001;

    private static native String getTypeString(int type);

    native void gatherLink(Sigar sigar, String name)
        throws SigarException;

    public String getTypeString() {
        return FileInfo.getTypeString(this.type);
    }

    public char getTypeChar() {
        switch (this.type) {
          case TYPE_DIR:
            return 'd';
          case TYPE_CHR:
            return 'c';
          case TYPE_BLK:
            return 'b';
          case TYPE_PIPE:
            return 'p';
          case TYPE_LNK:
            return 'l';
          case TYPE_SOCK:
            return 's';
          default:
            return '-';
        }
    }

    public String getName() {
        return this.name;
    }

    public int hashCode() {
        return this.name.hashCode();
    }

    public boolean equals(Object o) {
        return o.equals(this.name);
    }

    private static native String getPermissionsString(long type);

    public String getPermissionsString() {
        return FileInfo.getPermissionsString(this.permissions);
    }

    private static native int getMode(long permissions);

    /**
     * Convert permissions bit mask to human readable number.
     * Example:
     * <code>MODE_UREAD|MODE_UWRITE|MODE_GREAD|MODE_WREAD</code>
     * converts to <code>644</code>.
     * @return The file permissions mode.
     */
    public int getMode() {
        return FileInfo.getMode(this.permissions);
    }

    public void enableDirStat(boolean value) {
        this.dirStatEnabled = value;
        if (value) {
            if (this.type != TYPE_DIR) {
                throw new IllegalArgumentException(this.name + " is not a directory");
            }

            try {
                if (this.stat == null) {
                    this.stat = this.sigar.getDirStat(this.name);
                }
                else {
                    this.stat.gather(this.sigar, this.name);
                }
            } catch (SigarException e) {
                //ok for now
            }
        }
    }

    private class Diff {
        private String attr, old, cur;

        Diff(String attr, String old, String cur) {
            this.attr = attr;
            this.old = old;
            this.cur = cur;
        }

        Diff(String attr, int old, int cur) {
            this(attr,
                 String.valueOf(old),
                 String.valueOf(cur));
        }

        Diff(String attr, long old, long cur) {
            this(attr,
                 String.valueOf(old),
                 String.valueOf(cur));
        }

        public String toString() {
            return this.attr + ": " +
                this.old + "|" + this.cur;
        }
    }

    private StringBuffer format(ArrayList changes) {
        StringBuffer sb = new StringBuffer();

        if (changes.size() == 0) {
            return sb;
        }
        
        int size = changes.size();
        for (int i=0; i<size; i++) {
            sb.append('{').append(changes.get(i)).append('}');
        }

        return sb;
    }

    private static String formatDate(long time) {
        return DATE_FORMAT.format(new Date(time));
    }

    public String diff() {
        if (this.oldInfo == null) {
            return "";
        }
        return diff(this.oldInfo);
    }

    public String diff(DirStat stat) {
        DirStat thisStat = this.stat;
        ArrayList changes = new ArrayList();

        if (thisStat.files != stat.files) {
            changes.add(new Diff("Files",
                                 stat.getFiles(),
                                 thisStat.getFiles()));
        }

        if (thisStat.subdirs != stat.subdirs) {
            changes.add(new Diff("Subdirs",
                                 stat.getSubdirs(),
                                 thisStat.getSubdirs()));
        }

        if (thisStat.symlinks != stat.symlinks) {
            changes.add(new Diff("Symlinks",
                                 stat.getSymlinks(),
                                 thisStat.getSymlinks()));
        }

        if (thisStat.chrdevs != stat.chrdevs) {
            changes.add(new Diff("Chrdevs",
                                 stat.getChrdevs(),
                                 thisStat.getChrdevs()));
        }

        if (thisStat.blkdevs != stat.blkdevs) {
            changes.add(new Diff("Blkdevs",
                                 stat.getBlkdevs(),
                                 thisStat.getBlkdevs()));
        }

        if (thisStat.sockets != stat.sockets) {
            changes.add(new Diff("Sockets",
                                 stat.getSockets(),
                                 thisStat.getSockets()));
        }

        if (thisStat.total != stat.total) {
            changes.add(new Diff("Total",
                                 stat.getTotal(),
                                 thisStat.getTotal()));
        }

        return format(changes).toString();
    }

    public String diff(FileInfo info) {
        ArrayList changes = new ArrayList();

        if (this.getMtime() != info.getMtime()) {
            changes.add(new Diff("Mtime",
                                 formatDate(info.getMtime()),
                                 formatDate(this.getMtime())));
        }
        else if (this.getCtime() != info.getCtime()) {
            changes.add(new Diff("Ctime",
                                 formatDate(info.getCtime()),
                                 formatDate(this.getCtime())));
        }
        else {
            //no point in checking the rest if all times are the same.
            //or should we include atime in the diff?
            return "";
        }

        if (this.getPermissions() != info.getPermissions()) {
            changes.add(new Diff("Perms",
                                 info.getPermissionsString(),
                                 this.getPermissionsString()));
        }

        if (this.getType() != info.getType()) {
            changes.add(new Diff("Type",
                                 info.getTypeString(),
                                 this.getTypeString()));
        }

        if (this.getUid() != info.getUid()) {
            changes.add(new Diff("Uid",
                                 info.getUid(),
                                 this.getUid()));
        }

        if (this.getGid() != info.getGid()) {
            changes.add(new Diff("Gid",
                                 info.getGid(),
                                 this.getGid()));
        }

        if (this.getSize() != info.getSize()) {
            changes.add(new Diff("Size",
                                 info.getSize(),
                                 this.getSize()));
        }

        if (!OperatingSystem.IS_WIN32) {
            if (this.getInode() != info.getInode()) {
                changes.add(new Diff("Inode",
                                     info.getInode(),
                                     this.getInode()));
            }

            if (this.getDevice() != info.getDevice()) {
                changes.add(new Diff("Device",
                                     info.getDevice(),
                                     this.getDevice()));
            }

            if (this.getNlink() != info.getNlink()) {
                changes.add(new Diff("Nlink",
                                     info.getNlink(),
                                     this.getNlink()));
            }
        }

        StringBuffer sb = format(changes);
        if (this.dirStatEnabled) {
            sb.append(diff(info.stat));
        }

        return sb.toString();
    }

    public FileInfo getPreviousInfo() {
        return this.oldInfo;
    }

    public boolean modified()
        throws SigarException,
               SigarFileNotFoundException {

        if (this.oldInfo == null) {
            this.oldInfo = new FileInfo();
            if (this.dirStatEnabled) {
                this.oldInfo.stat = new DirStat();
            }
        }
        copyTo(this.oldInfo);
        if (this.dirStatEnabled) {
            this.stat.copyTo(this.oldInfo.stat);
        }

        stat();

        return this.mtime != oldInfo.mtime;
    }

    public boolean changed()
        throws SigarException,
               SigarFileNotFoundException {

        return modified() || (this.ctime != oldInfo.ctime);
    }

    public void stat()
        throws SigarException,
               SigarFileNotFoundException {

        long mtime = this.mtime;

        if (this.lstat) {
            this.gatherLink(this.sigar, this.name);
        }
        else {
            this.gather(this.sigar, this.name);
        }

        if (this.dirStatEnabled &&
            (mtime != this.mtime)) //no need to fetch stat if unmodified.
        {
            this.stat.gather(this.sigar, this.name);
        }
    }

    private static FileInfo fetchInfo(Sigar sigar, String name,
                                      boolean followSymlinks)
        throws SigarException {

        FileInfo info = new FileInfo();

        try {
            if (followSymlinks) {
                info.gather(sigar, name);
                info.lstat = false;
            }
            else {
                info.gatherLink(sigar, name);
                info.lstat = true;
            }
        } catch (SigarException e) {
            e.setMessage(name + ": " + e.getMessage());
            throw e;
        }

        info.sigar = sigar;
        info.name = name;
        return info;
    }

    static FileInfo fetchFileInfo(Sigar sigar, String name)
        throws SigarException {

        return fetchInfo(sigar, name, true);
    }

    static FileInfo fetchLinkInfo(Sigar sigar, String name)
        throws SigarException {

        return fetchInfo(sigar, name, false);
    }
}
