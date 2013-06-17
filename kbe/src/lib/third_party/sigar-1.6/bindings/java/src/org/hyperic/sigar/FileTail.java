/*
 * Copyright (c) 2006-2007, 2009 Hyperic, Inc.
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

import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import org.apache.log4j.Logger;

public abstract class FileTail extends FileWatcher {

    public static final String PROP_USE_SUDO =
        "sigar.tail.sudo";

    private boolean useSudo = 
        "true".equals(System.getProperty(PROP_USE_SUDO));

    private static final Logger log = 
        SigarLog.getLogger(FileTail.class.getName());

    private static final boolean isDebug = log.isDebugEnabled();

    public abstract void tail(FileInfo info, Reader reader);

    public FileTail(Sigar sigar) {
        super(sigar);
    }

    public void useSudo(boolean useSudo) {
        this.useSudo = useSudo;
    }

    static void error(String name, Throwable exc) {
        String msg = name + ": " + exc.getMessage(); 
        log.error(msg, exc);
    }

    public void onChange(FileInfo info) {
        InputStream in = null;
        Reader reader = null;
        String name = info.getName();

        try {
            if (this.useSudo) {
                in = new SudoFileInputStream(name);
            }
            else {
                in = new FileInputStream(name);
            }

            long offset = getOffset(info);

            if (offset > 0) {
                in.skip(offset); //use InputStream to skip bytes
            }
            reader = new InputStreamReader(in);
            tail(info, reader);
        } catch (IOException e) {
            error(name, e);
        } finally {
            if (reader != null) {
                try { reader.close(); } catch (IOException e) { }
            }
            if (in != null) {
                try { in.close(); } catch (IOException e) { }
            }
        }
    }

    public FileInfo add(String file)
        throws SigarException {
        FileInfo info = super.add(file);
        if (isDebug) {
            log.debug("add: " + file + "=" + info);
        }
        return info;
    }

    protected boolean changed(FileInfo info)
        throws SigarException,
               SigarFileNotFoundException {

        return
            info.modified() ||
            (info.getPreviousInfo().size != info.size);
    }

    private long getOffset(FileInfo current) {
        FileInfo previous = current.getPreviousInfo();

        if (previous == null) {
            if (isDebug) {
                log.debug(current.getName() + ": first stat");
            }
            return current.size;
        }

        if (current.inode != previous.inode) {
            if (isDebug) {
                log.debug(current.getName() + ": file inode changed");
            }
            return -1;
        }

        if (current.size < previous.size) {
            if (isDebug) {
                log.debug(current.getName() + ": file truncated");
            }
            return -1;
        }

        if (isDebug) {
            long diff = current.size - previous.size;
            log.debug(current.getName() + ": " + diff + " new bytes");
        }

        return previous.size;
    }
}
