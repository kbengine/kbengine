###v0.7.0

	2015/11/25

	��������ƣ�
		�ʼ�������������
		����API�ĵ�
		����interfaces��interfacesʹ��python������չ
		loginapp֧�ֽű���,����չ�ű������Ƶ�½�Ŷӹ��ܺͿ����˺ŵĵ�½����Ϊ
		deregisterFileDescriptor����ΪderegisterReadFileDescriptor
		proxy��destroy��ʱ֪ͨ�ͻ��˱��������߳�
		֧����ĳ��baseapp��cellapp�����븺�ؾ��⣨KBEngine.setAppFlags��KBEngine.getAppFlags��
		�����µ�API��Entity.getRandomPoints���������ȡĿ���������Χnavigate�ɵ����ָ������������㣨������NPC����ƶ���������Ʒ�������ȣ���
		
	BUG������
		����loginapp��http�ص��˿ڷ���ҳ��ʱ��������
		����:һ��entity�������BOOL��Ҳ����UINT8����Ȼ���˳����������ĳ���INT8������������Ժ����ݱ����bug(#263)
		�����ڷ������ϲ�����ĳʵ���ʱ�򣬿ͻ�������ʵ�巽���������crash
		��ֹAPP���˳�ʱ����־û��ͬ���꣨ͬ����logger����



###v0.6.21

	2015/10/26

	��������ƣ�
		��ԭ�е�CLIENYT_TYPE_PC��ֳ�CLIENYT_TYPE_WIN��CLIENYT_TYPE_LlINUX��CLIENYT_TYPE_MAC
		Entitydef��ժҪ��鲻����ͻ������Ͱ���һ������ͻ����ύ��ժҪ���飬û���ύ��ͻ��˶��Լ���Э����ȷ�Ը�������ͻ����ϸ�ӷ�����Զ�̵���Э�飬�����ϲ��������⣩
		��׼��һЩЭ������



###v0.6.20

	2015/10/23

	��������ƣ�
		�����µ�API��Entity.addYawRotator
		����API�ĵ�
		vs2010��Ŀ������vs2013
		ʵ�����������Ա������(#259)
		addSpaceGeometryMapping����������ָ����������navmesh��ĳ��layer��(#240)
		�������ݿ��ѯ�ӿڣ����õ�֧�ֲ�ͬ��ʽ�����ݿ���չ

	BUG������
		����email�����ʼ���������
		����ָ��FIXED_DICT���ʹ浵�ֶβ��Ե�����(#255)
		����dbmgr�����interfaces���ӵ�����
		����moveToPoint�Ĳ���distance����0ʱ��ʵ�����Ŀ�ĵ�С��distanceʱʵ������෴�ĵط�����



###v0.6.1

	2015/6/1

	��������ƣ�
		���ݿ�����ϵͳ����ID��Ϊuint64����
		���ӷ�����л���ͼʱ�ص�onEnterSpace/onLeaveSpace
		���ͻ��������÷����Э��ʱ��������flagsҲ�·����ͻ��ˣ������������͵��ж�
		һЩ�ṹ����

	BUG������
		����ĳ�������ʵ�����ٲ�δ֪ͨ�ͻ������ٵ�����
		����ĳЩ�����teleport�󣬿ͻ���û�иı�ʵ��λ��



###v0.6.0

	2015/5/3

	��������ƣ�
		Websocket FRC6544Э��ͨѶ����
		cluster_controller.py����Ⱥ���ƣ���installer.py����װ���֣���������
		assetsĬ�ϲ�����scripts/clientĿ¼�����ⲻ��Ҫ�����(�ƶ���OGRE���ӽű�Ŀ¼��)
		����API�ĵ�
		һЩ�ṹ����

	BUG������
		����bots�����˳�ʱcrash����
		�����ڽ��̷�æ״̬����ʱ���ܺܺõİ�ȫ�ط�
		����FixedDict����δ�ͷ�����



###v0.5.0

	2015/4/18

	��������ƣ�
		�Ż����ִ���ṹ
		�����µ�API��getClientDatas���������ڽű��ж�ȡ�ͻ��˵�½ʱ������������
		����API��getAoiRadius��getAoiHystArea
		ͳһ������ͬ�������˳����roll��pitch��yaw
		bots����entryScriptFile���� 

	BUG������
		Cellapp�ϵ�Exposed��������һ�������õ��Ĳ����ǵ����ߵ�ID
		����BASE_AND_CLIENT�������ڿͻ���ʵ�崴��֮���ͬ������
		�޸��ͻ���ע���˺�ʱ�ϱ������ݹ�������dbmgr���������



###v0.4.20

	2015/3/13

	��������ƣ�
		�Ż����ִ���ṹ
		����logger�����ٶ�
		API�ĵ�����

	BUG������
		����windows��genUUID64���ܲ���ȷ������
		����bots��ĳЩ�����µ�����־����û���ͷŵ�����



###v0.4.10

	2015/2/10

	��������ƣ�

	    ʵ���Զ����ع��ܣ��÷���API�ֲᣬbaseapp��writeToDB���֣�
	    onBaseAppReady��onReadyForLogin�ص�����������������Ϊbool���Ƿ�Ϊ��һ��������baseapp��
	    ���ӽ����������� --gus�������http://www.kbengine.org/cn/docs/startup_shutdown.html
	    ɾ��������������--grouporder��--globalorder����ֵ�ɳ����ڲ��Զ�������ͨ�����������ڽű��п��Ի��KBE_COMPONENTID��KBE_BOOTIDX_GLOBAL��KBE_BOOTIDX_GROUP
	    API�ĵ�����

	BUG������

	    ����teleport��ghost��Ϣ��������
	    ����teleport��pitch���������



###v0.4.0

	2015/1/23

        ���������ƣ�

		����ģ���Ż�����Ҫ���send�Լ�һЩ�ṹ���棩
		���Ӷ�TCP���ʹ��ڵ����ã��Ա��һЩ������п���
		���ؾ���ģ�������Cellapp�ڴ�����̬��������ʱ�ܸ��õľ��⸺�أ�
		��Ϣ����ģ�����
		
	BUG������
		
		����EntityDef��������һЩ�����def�ı��MD5��δ�仯������
		����Cellapp��ʼ��ʱִ����������Ҫ�����������Ĵ���
		����DBMgr�����ˣ�interfaces��logger���̵��Զ��ر�



###v0.3.0
	
	2014/12/30

	���������ƣ�

	    ����API�ĵ�
	    GUIConsole����log��������˵ȹ���
	    ����cluster_controller��
	    ������installer����
	    billingsystem����interfaces��kbmachine����Ϊmachine��messagelog����Ϊlogger
	    ����Ĭ�ϵ���Ŀ�ʲ�Ŀ¼"assets"�����û�����û����������潫�Զ��Ӹ�Ŀ¼Ѱ�Ҹ��ʲ�Ŀ¼
	    ԭ����demo�ʲ�Ŀ¼��Ǩ�Ƶ���������Ŀ�У��ھ����demo��ʹ��git submodule���
	    ��������С����

	BUG������

	    ����BASE_AND_CLIENT�����ͱ��޸ĺ�û��ͬ�����ͻ��˵�����
	    ����CLIENT_TYPE_MOBILE����Ҫ���ж�entitydef�Ĵ���
	    ����ʵ����ת������ghost���ƴ�������Ϣ��������



###v0.2.15

	2014/12/6

	��������ƣ�
	    �ű����ַ����������л������Ż��� ���Լ���һ���ڴ濽��
	    ��ǿGUIConsole��̽�⹦��
	    �Բ��ִ���ṹ���е���

	BUG������
	    ����AOI���������һЩ״̬���ҵ�����
	    globalorderid��Ӧ��ʹ��int32����Ŀǰint8�� �������������۵Ľ�������
	    ������Ŀ¼��û��log4j.logʱ��ʾ������Ϣ

