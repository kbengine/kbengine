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

import java.util.Collection;
import java.util.Vector;

public class MetaBase extends Win32
{
    private static int IIS_MD_SERVER_BASE             = 1000;
    private static int IIS_MD_HTTP_BASE               = 2000;

    /* NOTE:  This is only a partial list of the information that can 
     * get from the metabase.
     *
     * These properties are applicable to both HTTP and FTP virtual
     * servers
     */
    public static int MD_SERVER_COMMAND              = IIS_MD_SERVER_BASE+12;
    public static int MD_CONNECTION_TIMEOUT          = IIS_MD_SERVER_BASE+13;
    public static int MD_MAX_CONNECTIONS             = IIS_MD_SERVER_BASE+14;
    public static int MD_SERVER_COMMENT              = IIS_MD_SERVER_BASE+15;
    public static int MD_SERVER_STATE                = IIS_MD_SERVER_BASE+16;
    public static int MD_SERVER_AUTOSTART            = IIS_MD_SERVER_BASE+17;
    public static int MD_SERVER_SIZE                 = IIS_MD_SERVER_BASE+18;
    public static int MD_SERVER_LISTEN_BACKLOG       = IIS_MD_SERVER_BASE+19;
    public static int MD_SERVER_LISTEN_TIMEOUT       = IIS_MD_SERVER_BASE+20;
    public static int MD_DOWNLEVEL_ADMIN_INSTANCE    = IIS_MD_SERVER_BASE+21;
    public static int MD_LEVELS_TO_SCAN              = IIS_MD_SERVER_BASE+22;
    public static int MD_SERVER_BINDINGS             = IIS_MD_SERVER_BASE+23;
    public static int MD_MAX_ENDPOINT_CONNECTIONS    = IIS_MD_SERVER_BASE+24;
    public static int MD_SERVER_CONFIGURATION_INFO   = IIS_MD_SERVER_BASE+27;
    public static int MD_IISADMIN_EXTENSIONS         = IIS_MD_SERVER_BASE+28;

    public static int MD_LOGFILEDIRECTORY            = 4001;

    //  These properties are specific to HTTP and belong to the website
    public static int MD_SECURE_BINDINGS             = IIS_MD_HTTP_BASE+21;
    
    private int m_handle;
    private long pIMeta;

    public MetaBase()
    {
        pIMeta = MetaBaseInit();
    }

    public void close()
    {
        MetaBaseClose();
        MetaBaseRelease();
    }
    
    public void OpenSubKey(String subkey)
    {
        if (subkey.startsWith("/")) {
            MetaBaseOpenSubKeyAbs(subkey);
        } else {
            MetaBaseOpenSubKey(subkey);
        }
    }
    
    public int getIntValue(int datakey) throws Win32Exception
    {
        int iResult = 0;
        
        try {
            iResult = MetaBaseGetIntValue(datakey);
        } catch(Throwable t) {
            throw new Win32Exception("Error getting int value");
            //W32Service.throwLastErrorException();
        }
            
        return iResult;
    }
    
    public int getIntValue(int datakey, int defaultValue)
    {
        int iResult;
        
        try {
            iResult = this.getIntValue(datakey);
        } catch(Win32Exception e) {
            iResult = defaultValue;
        }
        
        return iResult;
    }
 
    public String getStringValue(int datakey) throws Win32Exception
    {
        String strResult = MetaBaseGetStringValue(datakey);
        
        if(strResult == null)
            //W32Service.throwLastErrorException();
            throw new Win32Exception("Error getting string value");

        return strResult;
    }
    
    public String getStringValue(int datakey, String defaultValue)
    {
        String  strResult;
        
        try {
            strResult = this.getStringValue(datakey);
        } catch(Win32Exception e) {
            strResult = defaultValue;
        }
        
        return strResult;
    }

    public String[] getMultiStringValue(int datakey) 
        throws Win32Exception
    {
        String[] strResult = MetaBaseGetMultiStringValue(datakey);
        
        return strResult;
    }

    public String[] getSubKeyNames()
    {
        Collection coll = new Vector();
        String     strName;
        
        for(int i = 0;(strName = MetaBaseEnumKey(i)) != null;i ++)
            coll.add(strName);
        
        return (String[])coll.toArray(new String[coll.size()]);
    }
    
    private final native long      MetaBaseInit();
    private final native void      MetaBaseClose();
    private final native void      MetaBaseRelease();
    private final native String    MetaBaseEnumKey(int index);
    private final native void      MetaBaseOpenSubKey(String subkey);
    private final native void      MetaBaseOpenSubKeyAbs(String subkey);
    private final native int       MetaBaseGetIntValue(int datakey);
    private final native String    MetaBaseGetStringValue(int datakey);
    private final native String[]  MetaBaseGetMultiStringValue(int datakey);

    public static void main(String args[]) {
        String key = "/LM/W3SVC";
        try {
            MetaBase mb = new MetaBase();

            mb.OpenSubKey(key);
            String logdir = mb.getStringValue(MD_LOGFILEDIRECTORY);
            System.out.println("Logdir: " + logdir);
            String keys[] = mb.getSubKeyNames();

            System.out.println("Listing IIS Web Sites");
            
            for (int i = 0; i < keys.length; i++) {
                int serverNum;
                try {
                    serverNum = Integer.parseInt(keys[i]);
                } catch (NumberFormatException e) {
                    continue;
                }

                MetaBase vhost = new MetaBase();
                vhost.OpenSubKey(key + "/" + serverNum);
                
                String[] bindings = 
                    vhost.getMultiStringValue(MD_SERVER_BINDINGS);
                
                String hostname = vhost.getStringValue(MD_SERVER_COMMENT);

                System.out.println("");
                System.out.println("Host: " + hostname);
                for (int j = 0; j < bindings.length; j++) {
                    System.out.println("Bindings: " + bindings[j]);
                }
       
                vhost.close();
            }
            
            mb.close();
        } catch (Win32Exception e) {
            System.out.println("Unable to query MetaBase for IIS Web Sites");
        }
    }
}
