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

package org.hyperic.sigar.shell;

import java.util.Stack;

import sun.misc.Signal;
import sun.misc.SignalHandler;

public class ShellIntHandler implements SignalHandler {

    private static ShellBase handlerShell;
    private static Stack handlers;

    public static void register(ShellBase shell) {
        handlerShell = shell;
        handlers = new Stack();

        Signal signal;
        try {
            signal = new Signal("INT");
        } catch (IllegalArgumentException e) {
            return; //e.g NetWare
        }

        try {
            Signal.handle(signal, new ShellIntHandler());
        } catch(Exception e) {
            //java -Xrs for example will throw IllegalArgumentException
        }
    }

    public void handle(Signal signal) {
        if (handlers.empty()) {
            handlerShell.shutdown();
            Runtime.getRuntime().halt(0);
        }
        else {
            SIGINT handler = (SIGINT)handlers.peek();
            handler.handleSIGINT();
        }
    }

    public static void push(SIGINT handler) {
        handlers.push(handler);
    }

    public static void pop() {
        handlers.pop();
    }
}
