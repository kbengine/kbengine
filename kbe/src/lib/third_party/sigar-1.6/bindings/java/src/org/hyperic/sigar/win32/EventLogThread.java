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

package org.hyperic.sigar.win32;

import java.io.IOException;
import java.util.Collections;
import java.util.HashSet;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;

import org.apache.log4j.Logger;

/**
 * A simple thread that runs forever monitoring the event log.
 */
public class EventLogThread implements Runnable {

    public static final int DEFAULT_INTERVAL = 60 * 1000;

    private static Logger logger =
        Logger.getLogger(EventLogThread.class.getName());

    private Thread thread = null;
    private static HashMap logs = new HashMap();
    private boolean shouldDie = false;
    private Set notifiers     = Collections.synchronizedSet(new HashSet());

    private String logName = EventLog.APPLICATION;
    private long interval  = DEFAULT_INTERVAL;

    /**
     * @deprecated
     */
    public static EventLogThread getInstance() {
        return getInstance(EventLog.APPLICATION);
    }

    public static EventLogThread getInstance(String name) {
        EventLogThread instance;

        synchronized (logs) {
            instance = (EventLogThread)logs.get(name);

            if (instance == null) {
                instance = new EventLogThread();
                instance.setLogName(name);
                logs.put(name, instance);
            }
        }

        return instance;
    }

    public static void closeInstances() {
        synchronized (logs) {
            for (Iterator it = logs.values().iterator();
                 it.hasNext();)
            {
                EventLogThread eventLogThread =
                    (EventLogThread)it.next();
                eventLogThread.doStop();
            }
            logs.clear();
        }
    }

    public void setInterval(long interval) {
        this.interval = interval;
    }

    public void setLogName(String logName) {
        this.logName = logName;
    }
    
    public synchronized void doStart() {
        if (this.thread != null) {
            return;
        }

        this.thread = new Thread(this, "EventLogThread");
        this.thread.setDaemon(true);
        this.thread.start();

        logger.debug(this.thread.getName() + " started");
    }

    public synchronized void doStop() {
        if (this.thread == null) {
            return;
        }
        die();
        this.thread.interrupt();
        logger.debug(this.thread.getName() + " stopped");
        this.thread = null;
    }

    public void add(EventLogNotification notifier) {
        this.notifiers.add(notifier);
    }

    public void remove(EventLogNotification notifier) {
        this.notifiers.remove(notifier);
    }

    private void handleEvents(EventLog log, int curEvent, int lastEvent)
    {
        for (int i = curEvent + 1; i <= lastEvent; i++) {

            EventLogRecord record;

            try {
                record = log.read(i);
            } catch (Win32Exception e) {
                logger.error("Unable to read event id " + i + ": " + e);
                continue;
            }

            synchronized (this.notifiers) {
                for (Iterator it = this.notifiers.iterator(); it.hasNext();)
                {
                    EventLogNotification notification =
                        (EventLogNotification)it.next();
                    if (notification.matches(record))
                        notification.handleNotification(record);
                }
            }
        }
    }

    public void run() {
        
        EventLog log = new EventLog();
        int curEvent;

        try {
            // Open the event log
            log.open(this.logName);

            curEvent = log.getNewestRecord();

            while (!shouldDie) {
                // XXX: Using the waitForChange() method would be a
                //      cleaner way to go, but we cannot interrupt
                //      a native system call.
                int lastEvent = log.getNewestRecord();
                if (lastEvent < curEvent) {
                    logger.debug(this.logName + " EventLog has changed, re-opening");
                    try { log.close(); } catch (Win32Exception e) {}
                    log.open(this.logName);
                    curEvent = log.getOldestRecord();
                    lastEvent = log.getNewestRecord();
                }

                if (lastEvent > curEvent) {
                    if (curEvent == -1) {
                        curEvent = 0; //log was cleared
                    }
                    handleEvents(log, curEvent, lastEvent);
                }

                curEvent = lastEvent;

                try {
                    Thread.sleep(this.interval);
                } catch (InterruptedException e) {
                }
            }
        } catch (Win32Exception e) {
            logger.error("Unable to monitor event log: ", e);
        } finally {
            try { log.close(); } 
            catch (Win32Exception e) {}
        }
    }

    public void die() {
        this.shouldDie = true;
    }

    public static void main(String[] args) {
        if (args.length == 0) {
            args = new String[] {
                EventLog.SYSTEM,
                EventLog.APPLICATION,
                EventLog.SECURITY
            };
        }

        EventLogNotification watcher =
            new EventLogNotification() {
                public boolean matches(EventLogRecord record) {
                    return true;
                }

                public void handleNotification(EventLogRecord record) { 
                    System.out.println(record);
                }
            };

        for (int i=0; i<args.length; i++) {
            String name = args[i];
            EventLogThread eventLogThread = 
                EventLogThread.getInstance(name);

            eventLogThread.doStart();
            eventLogThread.setInterval(1000);
            eventLogThread.add(watcher);
        }

        System.out.println("Press any key to stop");
        try {
            System.in.read();
        } catch (IOException e) { }

        EventLogThread.closeInstances();
    }
}
