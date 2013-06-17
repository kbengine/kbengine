/*
 * Copyright (c) 2006-2008 Hyperic, Inc.
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

package org.hyperic.sigar.win32.test;

import org.hyperic.sigar.test.SigarTestCase;
import org.hyperic.sigar.win32.EventLog;
import org.hyperic.sigar.win32.EventLogNotification;
import org.hyperic.sigar.win32.EventLogRecord;
import org.hyperic.sigar.win32.EventLogThread;
import org.hyperic.sigar.win32.Win32Exception;

public class TestEventLog extends SigarTestCase {

    public TestEventLog(String name) {
        super(name);
    }

    public void testOpenClose() throws Exception {
        EventLog log = new EventLog();

        // Try to close an event log that isn't open
        try {
            log.close();
            fail("Closing an unopened event log succeeded");
        } catch (Win32Exception e) {
            // OK
        }

        log.open(EventLog.APPLICATION);
        log.close();

        // Try to reopen using the System log
        log.open(EventLog.SYSTEM);
        log.close();
    }

    public void testGetNumberOfRecords() throws Exception {
        int numRecords;
        EventLog log = new EventLog();

        log.open(EventLog.APPLICATION);
        try {
            numRecords = log.getNumberOfRecords();
        } catch (Exception e) {
            fail("Unable to get the number of records");
        }

        log.close();
    }

    public void testGetOldestRecord() throws Exception {
        int oldestRecord;
        EventLog log = new EventLog();

        log.open(EventLog.APPLICATION);
        try {
            oldestRecord = log.getOldestRecord();
        } catch (Exception e) {
            fail("Unable to get the oldest event record");
        }

        log.close();
    }

    public void testGetNewestRecord() throws Exception {
        int newestRecord;
        EventLog log = new EventLog();

        log.open(EventLog.APPLICATION);
        try {
            newestRecord = log.getNewestRecord();
        } catch (Exception e) {
            fail("Unable to get the newest event record");
        }

        log.close();
    }

    private int readAll(String logname) throws Exception {
        int fail = 0, success = 0, max = 500;
        String testMax = System.getProperty("sigar.testeventlog.max");
        if (testMax != null) {
            max = Integer.parseInt(testMax);
        }
        EventLogRecord record;
        EventLog log = new EventLog();

        log.open(logname);
        if (log.getNumberOfRecords() == 0) {
            log.close();
            return 0; //else log.getOldestRecord() throws Exception
        }
        int oldestRecord = log.getOldestRecord();
        int numRecords = log.getNumberOfRecords();
        traceln("oldest=" + oldestRecord +
                ", total=" + numRecords +
                ", max=" + max);

        for (int i = oldestRecord; i < oldestRecord + numRecords; i++) {
            try {
                record = log.read(i);
                success++;
                if (success > max) {
                    break;
                }
            } catch (Win32Exception e) {
                fail++;
                traceln("Error reading record " + i + ": " +
                        e.getMessage());
            }
        }

        log.close();

        traceln("success=" + success + ", fail=" + fail);
        return success;
    }

    // Test reading all records
    public void testRead() throws Exception {
        int total = 0;
        String[] logs = EventLog.getLogNames();
        for (int i=0; i<logs.length; i++) {
            String msg = "readAll(" + logs[i] + ")"; 
            traceln(msg);
            total += readAll(logs[i]);
        }
        if (total == 0) {
            fail("No eventlog entries read");
        }
    }

    private class SSHEventLogNotification 
        implements EventLogNotification {

        public boolean matches(EventLogRecord record) {
            return record.getSource().equals("sshd");
        }

        public void handleNotification(EventLogRecord record) { 
            System.out.println(record);
        }
    }

    // Test event log thread
    public void testEventLogThread() throws Exception {
        EventLogThread thread =
            EventLogThread.getInstance(EventLog.APPLICATION);

        thread.doStart();

        SSHEventLogNotification notification =
            new SSHEventLogNotification();
        thread.add(notification);

        thread.doStop();
    }
}
