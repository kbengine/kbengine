/*
 * Copyright (c) 2006-2009 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
 * Copyright (c) 2010 VMware, Inc.
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

package org.hyperic.sigar.test;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;

import org.hyperic.sigar.ptql.ProcessQuery;
import org.hyperic.sigar.ptql.ProcessQueryFactory;
import org.hyperic.sigar.ptql.MalformedQueryException;

public class TestPTQL extends SigarTestCase {

    private static final String THIS_PROCESS = "Pid.Pid.eq=$$";
    private static final String OTHER_PROCESS = "Pid.Pid.ne=$$";
    private static final String JAVA_PROCESS = "State.Name.eq=java";
    private static final String OTHER_JAVA_PROCESS =
        JAVA_PROCESS + "," + OTHER_PROCESS;

    private ProcessQueryFactory qf;

    private static final String[] OK_QUERIES = {
        JAVA_PROCESS, //all java processs
        "Exe.Name.ew=java",   //similar
        JAVA_PROCESS + ",Exe.Cwd.eq=$user.dir", //process running this test
        JAVA_PROCESS + ",Exe.Cwd.eq=$PWD", //getenv
        "State.Name.ne=java,Exe.Cwd.eq=$user.dir", //parent(s) of process running this test
        "State.Name.sw=httpsd,State.Name.Pne=$1", //httpsd parent process
        "State.Name.ct=ssh", //anything ssh, "ssh", "ssh-agent", "sshd"
        JAVA_PROCESS + ",Args.-1.ew=AgentDaemon", //hq agents
        "Cred.Uid.eq=1003,State.Name.eq=java,Args.-1.ew=AgentDaemon", //my hq agent
        "Cred.Uid.gt=0,Cred.Uid.lt=1000", //range of users
        "Cred.Uid.eq=1003,Cred.Gid.eq=1003", //me
        "CredName.User.eq=dougm", //me
        "Time.Sys.gt=1000", //cpu hog
        "Fd.Total.gt=20", //lots of open files
        "Mem.Size.ge=10000000,Mem.Share.le=1000000", //memory hog
        "State.Name.eq=sshd,Cred.Uid.eq=0",
        "State.Name.eq=crond,Cred.Uid.eq=0",
        "State.State.eq=R", //processes in read state
        "Args.0.eq=sendmail: accepting connections",
        "Args.0.sw=sendmail: Queue runner@",
        "Args.1000.eq=foo",
        "Args.*.eq=org.apache.tools.ant.Main", //'*' == any arg
        "Args.*.ct=java", //'*' == any arg
        "Args.*.ew=sigar.jar", //'*' == any arg
        "Modules.*.re=libc|kernel",
        "Port.tcp.eq=80,Cred.Uid.eq=0", //root owned http port
        "Port.udp.eq=161,Cred.Uid.eq=0", //root owned snmp port
        "Port.tcp.eq=8080,Cred.Uid.eq=1003", //dougm owned jboss port
        "Pid.PidFile.eq=pid.file",
        "Pid.Pid.eq=1",
        THIS_PROCESS,
        "Pid.Service.eq=Eventlog", //compat -> Service.Name
        "Service.Name.eq=NOSUCHSERVICE",
        "Service.Name.ct=Oracle",
        "Service.DisplayName.re=DHCP|DNS",
        "Service.Path.ct=svchost",
        "Service.Exe.Ieq=inetinfo.exe",
        OTHER_JAVA_PROCESS, //all java procs cept this one
        "Cpu.Percent.ge=0.2",
        "State.Name.sw=java,Args.*.eq=org.jboss.Main", //jboss
        JAVA_PROCESS + ",Args.*.eq=com.ibm.ws.runtime.WsServer", //websphere
        JAVA_PROCESS + ",Args.-1.eq=weblogic.Server", //weblogic
        "State.Name.eq=perl,Args.*.eq=v", //testing w/ exp/fork.pl
    };

    //XXX current required 1.4+
    private static final String[] OK_RE_QUERIES = {
        "Args.-1.eq=weblogic.Server,Env.WEBLOGIC_CLASSPATH.re=.*weblogic.jar.*", //weblogic
        "State.Name.re=https?d.*|[Aa]pache2?$,State.Name.Pne=$1", //apache
        "State.Name.re=post(master|gres),State.Name.Pne=$1,Args.0.re=.*post(master|gres)$", //postgresql
        "State.Name.re=cfmx7|java,State.Name.Pne=$1,Args.*.ct=jrun.jar", //coldfusion
    };

    private static final String[] MALFORMED_QUERIES = {
        "foo",
        "State.Name",
        "State.Name.eq",
        "State.Namex.eq=foo",
        "Statex.Name.eq=foo",
        "State.Name.eqx=foo",
        "State.Name.Xeq=foo",
        "State.Name.eq=foo,Args.*.eq=$3",
        "State.Name.eq=$1",
        "State.State.eq=read",
        "Args.x.eq=foo",
        "Time.Sys.gt=x",
        "Pid.Pid.eq=foo",
        "Cpu.Percent.ge=x",
        "Port.foo.eq=8080",
        "Port.tcp.gt=8080",
        "Port.tcp.eq=http",
        "Cpu.Sys.ew=lots",
        "Service.Invalid.ew=.exe",
        "",
        null,
    };

    public TestPTQL(String name) {
        super(name);
    }

    protected void setUp() throws Exception {
        super.setUp();
        this.qf = new ProcessQueryFactory();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        this.qf.clear();
    }

    private int runQuery(Sigar sigar, String qs)
        throws MalformedQueryException,
               SigarException {

        ProcessQuery query;
        try {
            query = this.qf.getQuery(qs);
        } catch (MalformedQueryException e) {
            traceln("parse error: " + qs);
            throw e;
        }

        try {
            long[] pids = query.find(sigar);

            traceln(pids.length + " processes match: " + qs);

            if (qs.indexOf(OTHER_PROCESS) != -1) {
                long pid = sigar.getPid();
                for (int i=0; i<pids.length; i++) {
                    assertTrue(pid + "!=" + pids[i],
                               pid != pids[i]);
                }
            }
            return pids.length;
        } catch (SigarNotImplementedException e) {
            return 0;
        } catch (SigarException e) {
            traceln("Failed query: " + qs);
            throw e;
        }
    }

    public void testValidQueries() throws Exception {
        Sigar sigar = getSigar();

        assertTrue(THIS_PROCESS,
                   runQuery(sigar, THIS_PROCESS) == 1);

        int numProcs = runQuery(sigar, JAVA_PROCESS);
        int numOtherProcs = runQuery(sigar, OTHER_JAVA_PROCESS);
        String msg =
            JAVA_PROCESS +
            " [" + numProcs + "] vs. [" + numOtherProcs + "] "+
            OTHER_JAVA_PROCESS;
            
        //process may have gone away in between runQuery's above.
        //assertTrue(msg, numProcs != numOtherProcs);
        traceln(msg);

        for (int i=0; i<OK_QUERIES.length; i++) {
            String qs = OK_QUERIES[i];
            assertTrue(qs,
                       runQuery(sigar, qs) >= 0);
        }
    }

    public void testValidRegexQueries() throws Exception {
        for (int i=0; i<OK_RE_QUERIES.length; i++) {
            String qs = OK_RE_QUERIES[i];
            assertTrue(qs,
                       runQuery(getSigar(), qs) >= 0);
        }
    }

    public void testMalformedQueries() throws Exception {
        for (int i=0; i<MALFORMED_QUERIES.length; i++) {
            String qs = MALFORMED_QUERIES[i];
            try {
                runQuery(getSigar(), qs);
                fail("'" + qs + "' did not throw MalformedQueryException");
            } catch (MalformedQueryException e) {
                traceln(qs + ": " + e.getMessage());
                assertTrue(qs + " Malformed", true);
            }
        }
    }

    public void testSelf() throws Exception {
        Sigar sigar = getSigar();

        //should have eaten some cpu during this test
        String q = "Cpu.Percent.ge=0.01";
        ProcessQuery status = this.qf.getQuery(q);
        long pid = sigar.getPid();
        traceln(q + "=" + status.match(sigar, pid));
    }
}

