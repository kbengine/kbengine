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

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.ProcMem;

/**
 * Watch for changes in program memory usage.
 */
public class MemWatch {

    static final int SLEEP_TIME = 1000 * 10;

    public static void main(String[] args) throws Exception {
        Sigar sigar = new Sigar();

        if (args.length != 1) {
            throw new Exception("Usage: MemWatch pid");
        }

        long pid = Long.parseLong(args[0]);

        long lastTime = System.currentTimeMillis();

        ProcMem last = sigar.getProcMem(pid);

        while (true) {
            ProcMem cur = sigar.getProcMem(pid);
            
            StringBuffer diff = diff(last, cur);

            if (diff.length() == 0) {
                System.out.println("no change " +
                                   "(size=" +
                                   Sigar.formatSize(cur.getSize()) +
                                   ")");
            }
            else {
                long curTime = System.currentTimeMillis();
                long timeDiff = curTime - lastTime;
                lastTime = curTime;
                diff.append(" after " + timeDiff + "ms");
                System.out.println(diff);
            }

            last = cur;
            Thread.sleep(SLEEP_TIME);
        }
    }

    private static StringBuffer diff(ProcMem last, ProcMem cur) {
        StringBuffer buf = new StringBuffer();

        long diff;

        diff = cur.getSize() - last.getSize();
        if (diff != 0) {
            buf.append("size=" + diff);
        }

        diff = cur.getResident() - last.getResident();
        if (diff != 0) {
            buf.append(", resident=" + diff);
        }

        diff = cur.getShare() - last.getShare();
        if (diff != 0) {
            buf.append(", share=" + diff);
        }

        return buf;
    }
}

            
