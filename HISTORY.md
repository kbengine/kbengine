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

