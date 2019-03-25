namespace KBEngine
{
#if UNITY_EDITOR
    using UnityEngine;
    using UnityEditor;
    using System.Collections;
    using System;
    using System.IO;
    using KBEngine;

    public class ClientSDKUpdater : MonoBehaviour
    {
        string warnUpdateSDK = "";
        MemoryStream sdkFileStream = null;
        int downloadFiles = 0;
        string sdkPath = "";
        string sdkTempPath = "";
        string sdkBakPath = "";

        void Start()
        {
            string kbengineCoreFile = "KBEngine.cs";
            string[] res = System.IO.Directory.GetFiles(Application.dataPath, kbengineCoreFile, SearchOption.AllDirectories);
            sdkPath = res[0].Replace(kbengineCoreFile, "").Replace("\\", "/");
            sdkPath = sdkPath.Remove(sdkPath.Length - 1, 1);

            sdkTempPath = sdkPath + "_temp";
            sdkBakPath = sdkPath + "_bak";

            warnUpdateSDK = "Version does not match the server.\nClick to update KBEnginePlugin!\nPull from: " + KBEngineApp.app.getInitArgs().ip + ":" + KBEngineApp.app.getInitArgs().port;
            installEvents();

            GameObject[] objs = FindObjectsOfType(typeof(GameObject)) as GameObject[];
            foreach (GameObject child in objs)
            {
                if (!child.gameObject.GetComponent<Camera>() &&
                    !child.gameObject.GetComponent<KBEMain>() &&
                    !child.gameObject.GetComponent<ClientSDKUpdater>())
                {
                    child.gameObject.SetActive(false);
                }
            }
        }

        public virtual void installEvents()
        {
            Event.registerIn("onImportClientSDK", this, "onImportClientSDK");
        }

        protected virtual void OnDestroy()
        {
            KBEngine.Event.deregisterOut(this);
        }

        public void onImportClientSDK(int remainingFiles, string fileName, int fileSize, byte[] fileDatas)
        {
            if (sdkFileStream == null)
                sdkFileStream = MemoryStream.createObject();

            sdkFileStream.append(fileDatas, (uint)sdkFileStream.rpos, (uint)fileDatas.Length);

            warnUpdateSDK = "Download:" + fileName + " -> " + sdkFileStream.length() + "/" + fileSize + "bytes! " + (int)(((float)downloadFiles / (float)(downloadFiles + remainingFiles)) * 100) + "%";
            Debug.Log(warnUpdateSDK);

            if (sdkFileStream.length() == fileSize)
            {
                Debug.Log("onImportClientSDK: " + fileName + "->" + fileSize + "bytes success!");

                string path = Path.GetDirectoryName(sdkTempPath + "//" + fileName);
                if (!Directory.Exists(path))
                    Directory.CreateDirectory(path);

                StreamWriter sw;
                FileInfo t = new FileInfo(sdkTempPath + "//" + fileName);
                string data = System.Text.Encoding.UTF8.GetString(sdkFileStream.data(), 0, fileSize);
                sw = t.CreateText();
                sw.WriteLine(data);
                sw.Close();
                sw.Dispose();

                sdkFileStream.reclaimObject();
                sdkFileStream = null;
                downloadFiles += 1;

                if (remainingFiles == 0)
                {
                    warnUpdateSDK = "";
                    downloadFiles = 0;
                    replaceNewSDK();
                }
            }
        }

        void downloadSDKFromServer()
        {
            downloadFiles = 0;

            if (Directory.Exists(sdkTempPath))
                Directory.Delete(sdkTempPath, true);

            Directory.CreateDirectory(sdkTempPath);

                if (sdkFileStream != null)
            {
                sdkFileStream.reclaimObject();
                sdkFileStream = null;
            }

            // kbcmd options
            string tool_options = "Unity";
            string callbackIP = "";
            UInt16 callbackPort = 0;
            int clientWindowSize = (int)KBEngineApp.app.getInitArgs().TCP_RECV_BUFFER_MAX;

            Bundle bundle = Bundle.createObject();
            bundle.newMessage(Messages.messages["Loginapp_importClientSDK"]);
            bundle.writeString(tool_options);
            bundle.writeInt32(clientWindowSize);
            bundle.writeString(callbackIP);
            bundle.writeUint16(callbackPort);
            bundle.send(KBEngineApp.app.networkInterface());
        }

        void replaceNewSDK()
        {
           System.IO.Directory.Move(sdkPath, sdkBakPath);
            System.IO.Directory.Move(sdkTempPath, sdkPath);

            // 删除旧的SKD文件夹
            Directory.Delete(sdkBakPath, true);

            EditorApplication.isPlaying = false;
            AssetDatabase.Refresh();
        }

        void OnGUI()
        {
            if (warnUpdateSDK.Length > 0)
            {
                GUI.contentColor = Color.red;
                GUI.backgroundColor = Color.red;

                if (GUI.Button(new Rect(Screen.width * 0.25f, Screen.height * 0.4f, Screen.width * 0.5f, Screen.height * 0.2f), warnUpdateSDK))
                {
                    // 从服务器下载新的SDK
                    downloadSDKFromServer();
                }
            }
        }

        void Update()
        {


        }
    }
#endif
}
