/*
 * Copyright (c) 2006 Hyperic, Inc.
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

import java.io.File;

import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

public abstract class FileWatcher {

    private Sigar sigar;
    private long interval = 0;
    private long lastTime = 0;
    private Set files =
        Collections.synchronizedSet(new HashSet());

    public abstract void onChange(FileInfo info);

    public void onNotFound(FileInfo info) {
    }

    public void onException(FileInfo info, SigarException e) {
    }

    public FileWatcher(Sigar sigar) {
        this.sigar = sigar;
    }

    public void setInterval(long interval) {
        this.interval = interval;
    }

    public long getInterval() {
        return this.interval;
    }

    public FileInfo add(File file) 
        throws SigarException {
        return add(file.getAbsolutePath());
    }

    public FileInfo add(String file)
        throws SigarException {
        FileInfo info = this.sigar.getFileInfo(file);
        this.files.add(info);
        return info;
    }

    public void add(File[] files)
        throws SigarException {
        for (int i=0; i<files.length; i++) {
            add(files[i]);
        }
    }

    public void add(String[] files)
        throws SigarException {
        for (int i=0; i<files.length; i++) {
            add(files[i]);
        }
    }
    
    public void remove(File file) {
        remove(file.getAbsolutePath());
    }

    public void remove(String file) {
        FileInfo info = new FileInfo();
        info.name = file;
        this.files.remove(info);
    }

    public void clear() {
        this.files.clear();
    }

    public Set getFiles() {
        return this.files;
    }

    protected boolean changed(FileInfo info)
        throws SigarException,
               SigarFileNotFoundException {

        return info.changed();
    }

    public void check() {
        if (this.interval != 0) {
            long timeNow = System.currentTimeMillis();
            long timeDiff = timeNow - this.lastTime;

            if (timeDiff < this.interval) {
                return;
            }

            this.lastTime = timeNow;
        }

        synchronized (this.files) {
            for (Iterator it = this.files.iterator();
                 it.hasNext();)
            {
                FileInfo info = (FileInfo)it.next();

                try {
                    if (changed(info)) {
                        this.onChange(info);
                    }
                } catch (SigarFileNotFoundException e) {
                    this.onNotFound(info);
                } catch (SigarException e) {
                    this.onException(info, e);
                }
            }
        }
    }
}
