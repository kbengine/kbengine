// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.


using UnrealBuildTool;
using System.IO;
using System.Collections.Generic;

public class KBEnginePlugins : ModuleRules
{
    public KBEnginePlugins(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnableUndefinedIdentifierWarnings = false;

        string CryptoPPPath = Target.UEThirdPartySourceDirectory + "CryptoPP/5.6.5/lib/";
        string[] PrivateModules = new string[] { "Slate", "SlateCore", "Networking", "Sockets", "OpenSSL" };
        string[] PublicModules = new string[] { "Core", "CoreUObject", "Engine"};
        List<string> PublicModulesList = new List<string>(PublicModules);

        //if (Directory.Exists(CryptoPPPath))
        //{
        //    List<string> PrivateModuleList = new List<string>(PrivateModules);
        //    PrivateModuleList.Add("CryptoPP");
        //    PrivateModules = PrivateModuleList.ToArray();
        //}
        //else
        //{
        //   // PublicDefinitions.Add("KBENGINE_NO_CRYPTO");
        //   // PublicDefinitions.Add("_MINWINDEF_");
        //}


        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				"KBEnginePlugins/Engine", 
				"KBEnginePlugins/Scripts",
			}
			);

        if (Target.bBuildEditor)
        {
            PublicModulesList.Add("UnrealEd");
        }

        PublicModules = PublicModulesList.ToArray();
        PublicDependencyModuleNames.AddRange(PublicModules);

        PrivateDependencyModuleNames.AddRange(PrivateModules);

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}
