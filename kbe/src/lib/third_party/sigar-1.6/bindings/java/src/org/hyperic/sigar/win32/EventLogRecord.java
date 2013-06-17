/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
 * Copyright (c) 2009 VMware, Inc.
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

import java.util.Date;

/**
 * Class to represent event log records
 */
public class EventLogRecord {

    private static final String NA = "N/A";

    long recordNumber;
    long timeGenerated; 
    long timeWritten;
    long eventId;

    short eventType;
    short category;

    String categoryString;
    String source;
    String computerName;
    String user;
    String message;
    String logName;

    EventLogRecord() {}

    /**
     * @return Event log name which generated the event
     */
    public String getLogName() {
        return this.logName;
    }

    void setLogName(String logName) {
        this.logName = logName;
    }

    /* Get the record number for this event entry */
    public long getRecordNumber() {
        return recordNumber;
    }

    /**
     * Get the time at which this entry was submitted. This time is 
     * measured in the number of seconds elapsed since 00:00:00 
     * January 1, 1970, Universal Coordinated Time. 
     */

    public long getTimeGenerated() {
        return this.timeGenerated;
    }

    /**
     * Get the time at which this entry was received by the service to be 
     * written to the logfile. This time is measured in the number of 
     * seconds elapsed since 00:00:00 January 1, 1970, Universal 
     * Coordinated Time.
     */
    public long getTimeWritten() {
        return this.timeWritten;
    }

    /**
     * Event identifier. The value is specific to the event source 
     * for the event, and is used with source name to locate a 
     * description string in the message file for the event source. 
     *
     * XXX: This is probably not needed
     */
    public long getEventId() {
        return this.eventId;
    }

    /**
     * Return the event type.  
     * See the EVENTLOG_* constants in the EventLog class
     */
    public short getEventType() {
        return this.eventType;
    }

    public String getEventTypeString() {
        switch (this.eventType) {
            case EventLog.EVENTLOG_ERROR_TYPE:
                return "Error";
            case EventLog.EVENTLOG_WARNING_TYPE:
                return "Warning";
            case EventLog.EVENTLOG_INFORMATION_TYPE:
                return "Information";
            case EventLog.EVENTLOG_AUDIT_SUCCESS:
                return "Success Audit";
            case EventLog.EVENTLOG_AUDIT_FAILURE:
                return "Failure Audit";
            default:
                return "Unknown";
        }
    }

    /**
     * Get the category for this event.
     * The meaning of this value depends on the event source.
     */
    public short getCategory() {
        return this.category;
    }

    /**
     * Get the formatted string for the category.
     */
    public String getCategoryString() {
        if (this.categoryString != null) {
            return this.categoryString.trim();
        }
        if (this.category == 0) {
            return "None";
        }
        else {
            return "(" + this.category + ")";
        }
    }

    /**
     * Get the application which triggered the event
     */
    public String getSource() {
        return this.source;
    }

    /**
     * Get the machine name where the event was generated
     */
    public String getComputerName() {
        return this.computerName;
    }

    /**
     * Get the user who generated the event.  May be null if no user is
     * associated with the event.
     */
    public String getUser() {
        return this.user;
    }

    private String getUserString() {
        if (this.user == null) {
            return NA;
        }
        return this.user;
    }

    /**
     * Get the message for the event.
     */
    public String getMessage() {
        return this.message;
    }

    /**
     * @deprecated
     */
    public String getStringData() {
        return getMessage();
    }

    /**
     * For debugging
     */
    public String toString()
    {
        return
        "[" + getEventTypeString() + "] " +
        "[" + new Date(getTimeGenerated() * 1000) + "] " +
        "[" + getSource() + "] " +
        "[" + getCategoryString() + "] " +
        "[" + (getEventId() & 0xFFFF) + "] " +
        "[" + getUserString() + "] " +
        "[" + getComputerName() + "] " +
        getMessage();
    }
}

