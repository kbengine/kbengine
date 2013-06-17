/*
 * Copyright (c) 2007, 2009 Hyperic, Inc.
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

import org.hyperic.sigar.win32.EventLog;
import org.hyperic.sigar.win32.EventLogNotification;
import org.hyperic.sigar.win32.EventLogRecord;
import org.hyperic.sigar.win32.EventLogThread;
import org.hyperic.sigar.win32.Win32Exception;

public class EventLogTail {

    private static void tail(String name, Tail tail) throws Win32Exception {
        EventLog log = new EventLog();
        log.open(name);
        int max = log.getNumberOfRecords();
        if (tail.number < max) {
            max = tail.number;
        }
        int last = log.getNewestRecord()+1;
        int first = last - max;

        for (int i=first; i<last; i++) {
            EventLogRecord record = log.read(i);
            System.out.println(record);
        }
        log.close();
    }

    private static class TailNotification implements EventLogNotification {
        public void handleNotification(EventLogRecord event) {
            System.out.println(event);
        }

        public boolean matches(EventLogRecord event) {
            return true;
        }
    }

    public static void main(String[] args) throws Exception {
        Tail tail = new Tail();
        tail.parseArgs(args);

        if (tail.files.size() == 0) {
            tail.files.add(EventLog.SYSTEM);
        }

        for (int i=0; i<tail.files.size(); i++) {
            String name = (String)tail.files.get(i);
            tail(name, tail);

            if (tail.follow) {
                TailNotification notifier = new TailNotification();
                EventLogThread thread = 
                    EventLogThread.getInstance(name);
                thread.add(notifier);
                thread.doStart();
            }
        }

        if (tail.follow) {
            System.in.read();
        }
    }
}
