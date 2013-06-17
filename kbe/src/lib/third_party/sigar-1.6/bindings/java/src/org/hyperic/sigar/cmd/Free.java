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

import org.hyperic.sigar.Mem;
import org.hyperic.sigar.Swap;
import org.hyperic.sigar.SigarException;

/**
 * Display amount of free and used memory in the system.
 */
public class Free extends SigarCommandBase {

    public Free(Shell shell) {
        super(shell);
    }

    public Free() {
        super();
    }

    public String getUsageShort() {
        return "Display information about free and used memory";
    }

    private static Long format(long value) {
        return new Long(value / 1024);
    }

    public void output(String[] args) throws SigarException {
        Mem mem   = this.sigar.getMem();
        Swap swap = this.sigar.getSwap();

        Object[] header = new Object[] { "total", "used", "free" };

        Object[] memRow = new Object[] {
            format(mem.getTotal()),
            format(mem.getUsed()),
            format(mem.getFree())
        };

        Object[] actualRow = new Object[] {
            format(mem.getActualUsed()),
            format(mem.getActualFree())
        };

        Object[] swapRow = new Object[] {
            format(swap.getTotal()),
            format(swap.getUsed()),
            format(swap.getFree())
        };

        printf("%18s %10s %10s", header);

        printf("Mem:    %10ld %10ld %10ld", memRow);

        //e.g. linux
        if ((mem.getUsed() != mem.getActualUsed()) ||
            (mem.getFree() != mem.getActualFree()))
        {
            printf("-/+ buffers/cache: " + "%10ld %10d", actualRow);
        }

        printf("Swap:   %10ld %10ld %10ld", swapRow);

        printf("RAM:    %10ls", new Object[] { mem.getRam() + "MB" });
    }

    public static void main(String[] args) throws Exception {
        new Free().processCommand(args);
    }
}
