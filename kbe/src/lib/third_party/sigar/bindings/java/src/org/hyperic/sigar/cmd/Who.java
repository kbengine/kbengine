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

package org.hyperic.sigar.cmd;

import java.util.Date;
import java.text.SimpleDateFormat;

import org.hyperic.sigar.SigarException;

public class Who extends SigarCommandBase {

    public Who(Shell shell) {
        super(shell);
    }

    public Who() {
        super();
    }

    public String getUsageShort() {
        return "Show who is logged on";
    }

    private String getTime(long time) {
        if (time == 0) {
            return "unknown";
        }
        String fmt = "MMM dd HH:mm";
        return new SimpleDateFormat(fmt).format(new Date(time));
    }

    public void output(String[] args) throws SigarException {
        org.hyperic.sigar.Who[] who = this.sigar.getWhoList();
        for (int i=0; i<who.length; i++) {
            String host = who[i].getHost();
            if (host.length() != 0) {
                host = "(" + host + ")";
            }
            printf(new String[] {
                who[i].getUser(),
                who[i].getDevice(),
                getTime(who[i].getTime() * 1000),
                host            
            });
        }
    }

    public static void main(String[] args) throws Exception {
        new Who().processCommand(args);
    }
}
