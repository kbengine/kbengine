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

package org.hyperic.sigar.cmd;

import java.io.IOException;
import java.io.BufferedReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.FileInfo;
import org.hyperic.sigar.FileTail;
import org.hyperic.sigar.FileWatcherThread;

/**
 * Display the last part of files to the standard output.
 */
public class Tail {

    public boolean follow;
    public int number = 10;
    public List files = new ArrayList();

    public void parseArgs(String args[]) throws SigarException {
        for (int i=0; i<args.length; i++) {
            String arg = args[i];
            if (arg.charAt(0) != '-') {
                this.files.add(arg);
                continue;
            }
            arg = arg.substring(1);
            if (arg.equals("f")) {
                this.follow = true;
            }
            else if (Character.isDigit(arg.charAt(0))) {
                this.number = Integer.parseInt(arg);
            }
            else {
                throw new SigarException("Unknown argument: " + args[i]);
            }
        }
    }

    public static void main(String[] args) throws SigarException {
        Sigar sigar = new Sigar();

        FileWatcherThread watcherThread = 
            FileWatcherThread.getInstance();

        watcherThread.doStart();

        watcherThread.setInterval(1000);

        FileTail watcher =
            new FileTail(sigar) {
                public void tail(FileInfo info, Reader reader) {
                    String line;
                    BufferedReader buffer =
                        new BufferedReader(reader);

                    if (getFiles().size() > 1) {
                        System.out.println("==> " +
                                           info.getName() +
                                           " <==");
                    }

                    try {
                        while ((line = buffer.readLine()) != null) {
                            System.out.println(line);
                        }
                    } catch (IOException e) {
                        System.out.println(e);                    
                    }
                }
            };

        for (int i=0; i<args.length; i++) {
            watcher.add(args[i]);
        }

        watcherThread.add(watcher);

        try {
            System.in.read();
        } catch (IOException e) { }

        watcherThread.doStop();
    }
}
