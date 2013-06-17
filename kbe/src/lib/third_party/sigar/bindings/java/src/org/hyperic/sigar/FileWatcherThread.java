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

import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

public class FileWatcherThread implements Runnable {

    public static final int DEFAULT_INTERVAL = 60 * 5 * 1000;

    private Thread thread = null;
    private static FileWatcherThread instance = null;
    private boolean shouldDie = false;
    private long interval = DEFAULT_INTERVAL;
    private Set watchers =
        Collections.synchronizedSet(new HashSet());

    public static synchronized FileWatcherThread getInstance() {
        if (instance == null) {
            instance = new FileWatcherThread();
        }

        return instance;
    }

    public synchronized void doStart() {
        if (this.thread != null) {
            return;
        }

        this.thread = new Thread(this, "FileWatcherThread");
        this.thread.setDaemon(true);
        this.thread.start();
    }

    public synchronized void doStop() {
        if (this.thread == null) {
            return;
        }
        die();
        this.thread.interrupt();
        this.thread = null;
    }

    public void setInterval(long interval) {
        this.interval = interval;
    }

    public long getInterval() {
        return this.interval;
    }

    public void add(FileWatcher watcher) {
        this.watchers.add(watcher);
    }

    public void remove(FileWatcher watcher) {
        this.watchers.remove(watcher);
    }

    public void run() {
        while (!shouldDie) {
            check();
            try {
                Thread.sleep(this.interval);
            } catch (InterruptedException e) {
            }
        }
    }

    public void die() {
        this.shouldDie = true;
    }

    public void check() {
        synchronized (this.watchers) {
            for (Iterator it = this.watchers.iterator();
                 it.hasNext();)
            {
                FileWatcher watcher = (FileWatcher)it.next();
                try {
                    watcher.check();
                } catch (Exception e) {
                    FileTail.error("Unexpected exception: " +
                                   e.getMessage(), e);
                }
            }
        }
    }
}
