 /*
 * 一些类似于思路来自于python的工具函数
 * write by penghuawei
 */

KBEngine = {
    COMPONENT_TYPE : {
        UNKNOWN_COMPONENT_TYPE	: 0,
        DBMGR_TYPE				: 1,
        LOGINAPP_TYPE			: 2,
        BASEAPPMGR_TYPE			: 3,
        CELLAPPMGR_TYPE			: 4,
        CELLAPP_TYPE			: 5,
        BASEAPP_TYPE			: 6,
        CLIENT_TYPE				: 7,
        MACHINE_TYPE			: 8,
        CONSOLE_TYPE			: 9,
        LOGGER_TYPE				: 10,
        BOTS_TYPE				: 11,
        WATCHER_TYPE			: 12,
        INTERFACES_TYPE			: 13,
        COMPONENT_END_TYPE		: 14,
    },
    
    COMPONENT_NAME2TYPE : {
        unknown    : 0,
        dbmgr      : 1,
        loginapp   : 2,
        baseappmgr : 3,
        cellappmgr : 4,
        cellapp    : 5,
        baseapp    : 6,
        client     : 7,
        machine    : 8,
        console    : 9,
        logger     : 10,
        bots       : 11,
        watcher    : 12,
        interfaces : 13,
    },
    
    COMPONENT_NAME : [
        "unknown",
        "dbmgr",
        "loginapp",
        "baseappmgr",
        "cellappmgr",
        "cellapp",
        "baseapp",
        "client",
        "machine",
        "console",
        "logger",
        "bots",
        "watcher",
        "interfaces",
    ],
    
    vt100reg : new RegExp("\33\\[.*?D|\33\\[.*?C|\33\\[.*?m"),
    
    // 对接收到的数据进行过滤，把vt100终端命令过滤掉
    filterConsoleCmd : function(data) {
        var d = data.replace(this.vt100reg, "");
        return d;
    },
    
    resetPyTickProfileData : function() {
        $.data.pyTickProfileDatas = [];
        $.data.lastPyTickProfileData = null;
        $.data.lastReceivedTickData = null;
    },
    
    newLastPyTickProfileData : function () {
        $.data.lastPyTickProfileData = {
            title   : "",
            head    : ["ncalls", "tottime", "percall", "cumtime", "percall", "filename:lineno(function)"],
            lines   : [],
            totcall : 0,
            tottime : 0,
        };
        return $.data.lastPyTickProfileData;
    },
    
    pushLastReceivedTickData : function (data) {
        if ($.data.lastReceivedTickData)
            $.data.lastReceivedTickData += data;
        else
            $.data.lastReceivedTickData = data;
    },
    
    parsePyTickProfileData : function (data,cmd) {
       // 把上次未结束的数据取出来继续处理
        if ($.data.lastReceivedTickData)
        {
            data = $.data.lastReceivedTickData + data;
            $.data.lastReceivedTickData = "";
        }

        if (data.length < 2)
        {
            $.data.lastReceivedTickData = data;
            return;
        }
        
        var ds = data.split("\r\n");

        if (data.slice(-2) != "\r\n")
        {
            $.data.lastReceivedTickData = ds.pop();
        }
            
        var tickData = $.data.lastPyTickProfileData;
        
        // 开始处理
        for (var i = 0; i < ds.length; i++)
        {
            line = pytools.strip(ds[i]);
            line = this.filterConsoleCmd(line);
            if (line == "")
            {
                if (tickData && tickData.lines.length > 0)
                {
                    $.data.pyTickProfileDatas.push(tickData);
                    tickData = null;
                }
                
                continue;
            }

            //获取pytickprofile的数据
            if ( cmd == 'pytickprofile') 
            {
                if (line.match(/^\d+ function calls.* in [0-9.]+ seconds/)) // tick profile start or end
                {
                    if (tickData)
                    {
                        //$.data.pyTickProfileDatas.push(tickData);
                    }
                    tickData = this.newLastPyTickProfileData();
                    
                    //ls = line.split(/^(\d+) function calls.* in ([0-9.]+) seconds/g);
                    tickData.title = line;
                    var vs = line.split(/^(\d+) function calls.* in ([0-9.]+) seconds/);
                    tickData.totcall = parseInt(vs[1]);
                    tickData.tottime = parseFloat(vs[2]);
                    continue;
                }
                
                if (!tickData)
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown title data: %s", line);
                    continue;
                }
                
                if (line.match(/^Ordered by:.*/))
                {
                    continue;
                }
                else if (line.match(/^ncalls[ \t]+tottime[ \t]+percall[ \t]+cumtime[ \t]+percall.*/))
                {
                    continue;
                }
                else if (line.match(/([0-9\/]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+(.*)/))
                {
                    // "1    0.001    0.001    0.001    0.001 {built-in method print}"  ->
                    // [ "", "1", "0.001", "0.001", "0.001", "0.001", "{built-in method print}", ""]
                    var l = line.split(/([0-9\/]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+(.*)/g)
                    tickData.lines.push( l.slice(1, 7) );
                }
                else
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown data: %s", line);
                }
            }

            //获取cprofile的数据
            if (cmd == 'cprofile') 
            {
                if (line.match(/^ncalls[ \t]+tottime[ \t]+percall[ \t]+cumtime[ \t]+percall.*/)) // tick profile start or end
                {
                    if (tickData)
                    {
                        //$.data.pyTickProfileDatas.push(tickData);
                    }
                    tickData = this.newLastPyTickProfileData();
                    tickData.title = line;
                    continue;
                }
                
                if (!tickData)
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown title data: %s", line);
                    continue;
                }
                else if (line.match(/^ncalls[ \t]+tottime[ \t]+percall[ \t]+cumtime[ \t]+percall.*/))
                {
                    continue;
                }
                else if (line.match(/([0-9\/]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+(.*)/))
                {
                    var l = line.split(/([0-9\/]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+(.*)/g)
                    tickData.lines.push( l.slice(1, 7) );
                }
                else
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown data: %s", line);
                }
                console.log("lines");
            }

            //获取pyprofile的数据
            if (cmd == 'pyprofile') 
            {
                if (line.match(/^\d+ function calls.* in [0-9.]+ seconds/)) // tick profile start or end
                {
                    if (tickData)
                    {
                        //$.data.pyTickProfileDatas.push(tickData);
                    }
                    tickData = this.newLastPyTickProfileData();
                    
                    //ls = line.split(/^(\d+) function calls.* in ([0-9.]+) seconds/g);
                    tickData.title = line;
                    var vs = line.split(/^(\d+) function calls.* in ([0-9.]+) seconds/);
                    tickData.totcall = parseInt(vs[1]);
                    tickData.tottime = parseFloat(vs[2]);
                    continue;
                }
                
                if (!tickData)
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown title data: %s", line);
                    continue;
                }
                
                if (line.match(/^Ordered by:.*/))
                {
                    continue;
                }
                else if (line.match(/^ncalls[ \t]+tottime[ \t]+percall[ \t]+cumtime[ \t]+percall.*/))
                {
                    var h = line.split(/^ncalls[ \t]+tottime[ \t]+percall[ \t]+cumtime[ \t]+percall.*/g)
                    tickData.head.push( h.slice(1, 7) );
                    continue;
                }
                else if (line.match(/([0-9\/]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+(.*)/))
                {
                    var l = line.split(/([0-9\/]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+(.*)/g)
                    tickData.lines.push( l.slice(1, 7) );
                }
                else
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown data: %s", line);
                }
            }
            if (cmd == 'eventprofile')
            {

                if (line.match(/Event Type:./)) // tick profile start or end
                {
                    if (tickData)
                    {
                        //$.data.pyTickProfileDatas.push(tickData);
                    }
                    tickData = this.newLastPyTickProfileData();
                    
                    tickData.title = line;
                    tickData.head = ["name", "count", "size"];
                    continue;
                }
                
                if (!tickData)
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown title data: %s", line);
                    continue;
                }
                
                else if (line.match(/^name[ \W]+count[ \W]+size/))
                {
                    
                    continue;
                }
                else if (line.match(/(.+)[ \W]+([0-9.]+)[ \W]+([0-9.])/))
                {
                    var l = line.split(/(.+)[ \W]+([0-9.]+)[ \W]+([0-9.])/g);
                    tickData.lines.push( l.slice(1, 4) );
                }
                else
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown data: %s", line);
                }
            }
            //获取networkprofile的数据
            if (cmd == 'networkprofile') 
            {
                if (line.match(/^name[ \t]+sent#[ \t]+size[ \t]+avg[ \t]+total#[ \t]+totalsize[ \t]+recv#[ \t]+size[ \t]+avg[ \t]+total#[ \t]+totalsize.*/)) // tick profile start or end
                {
                    if (tickData)
                    {
                        //$.data.pyTickProfileDatas.push(tickData);
                    }
                    tickData = this.newLastPyTickProfileData();
                    
                    tickData.title = line;
                    tickData.head = ["ncalls", "sent#", "size", "avg", "total#", "totalsize","recv#","size","avg","total#","totalsize"];
                    continue;
                }
                
                if (!tickData)
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown title data: %s", line);
                    continue;
                }

                if (line.match(/(.+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.])/))
                {
                    var l = line.split(/(.+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.]+)[ \t]+([0-9.])/g)
                    tickData.lines.push( l.slice(1, 12) );
                }
                else
                {
                    console.error("KBEngine::parsePyTickProfileData(), unknown data: %s", line);
                }
            }

        }
    },
    

};

