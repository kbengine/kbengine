/*!
 * 一些类似于思路来自于python的工具函数
 * write by penghuawei
 */

pytools = {
    range : function( first, end, step )
    {
        if (typeof step === 'undefined')
            step = 1;
        
        if (typeof end === 'undefined')
        {
            end = first;
            first = 0;
        }
        
        if (typeof first === 'undefined')
            return [];
        
        
        var v = [];
        for (var i = first; i < end; i++)
            v.push( i );
        return v;
    },

    array : function( elems, defaultVal )
    {
        var v = new Array( elems );
        for (var i = 0; i < elems; i++)
        	v[i] = defaultVal;
        return v;
    },

    lstrip : function( str1, str2 )
    {
        var spl = str2; 
        var typeName = typeof str2;
        if (typeName === 'undefined' || typeName === 'null')
            spl = "^[\r\n\t ]*";
        else
            spl = "^[" + str2 + "]*";
        
        return str1.replace(new RegExp(spl), "");
    },

    rstrip : function( str1, str2 )
    {
        var spl = str2; 
        var typeName = typeof str2;
        if (typeName === 'undefined' || typeName === 'null')
            spl = "[\r\n\t ]*$";
        else
            spl = "[" + str2 + "]*$";
        
        return str1.replace(new RegExp(spl), "");
    },

    strip : function( str1, str2 )
    {
        var spl = str2; 
        var typeName = typeof str2;
        if (typeName === 'undefined' || typeName === 'null')
            spl = "^[\r\n\t ]*|[\r\n\t ]*$";
        else
            spl = "^[" + str2 + "]*" + "[" + str2 + "]*$";
        
        return str1.replace(new RegExp(spl, "g"), "");
    },

};

