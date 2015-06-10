/*
 * main.cpp
 *
 *  Created on: 2014年4月15日
 *      Author: wenshengming
 */
#include "XMySqlConnector.h"
#include <iconv.h>

//	连接account数据库
int testcallprocedureandreturnout(XMySqlConnector& mysqlConnector)
{
	char name[32] = "nonamenovalue";

	//	读取 数据
	if (!mysqlConnector.InitBindParam())
	{
		return -1;
	}
	if (!mysqlConnector.BindParam(E_Data_Type_String, name, strlen(name), E_Param_Input))
	{
		return -1;
	}
	int64_t n64Accountid = 0;
	if (!mysqlConnector.BindParam(E_Data_Type_Int64, &n64Accountid, sizeof(int64_t), E_Param_Output))
	{
		return -1;
	}
	if (!mysqlConnector.SetProcedureName("addaccount"))
	{
		return -1;
	}
	if (!mysqlConnector.ExcuteSql())
	{
		return -1;
	}
	if (!mysqlConnector.FreeMoreResult())
	{
		return -1;
	}
	if (!mysqlConnector.GetOutputResult())
	{
		return -1;
	}

	return 0;
}

int testcallprocedurereadresult(XMySqlConnector& mysqlConnector)
{
	if (!mysqlConnector.InitBindParam())
	{
		return -1;
	}
	if (!mysqlConnector.SetProcedureName("getallaccount"))
	{
		return -1;
	}
	if (!mysqlConnector.ExcuteSql())
	{
		return -1;
	}
	if (!mysqlConnector.InitBindCol())
	{
		return -1;
	}
	int64_t n64Accountid = 0;
	char szaccountname[21]; memset(szaccountname, 0, sizeof(szaccountname));

	if (!mysqlConnector.BindCol(E_Data_Type_Int64, &n64Accountid, sizeof(int64_t)))
	{
		return -1;
	}
	if (!mysqlConnector.BindCol(E_Data_Type_String, szaccountname, 20))
	{
		return -1;
	}
	while(mysqlConnector.Fetch())
	{
		printf("Record:%lld, name:%s\n", n64Accountid, szaccountname);
		n64Accountid = 0;
		memset(szaccountname, 0, sizeof(szaccountname));
	}
	if (!mysqlConnector.FreeMoreResult())
	{
		return -1;
	}
	return 0;
}

int testmultiresultandreturnout(XMySqlConnector& mysqlConnector)
{
	//	返回的结果
	char szcharname[21]; memset(szcharname, 0, sizeof(szcharname));
	if (!mysqlConnector.InitBindParam())
	{
		return -1;
	}
	//if ()
	if (!mysqlConnector.SetProcedureName("multirecordtest"))
	{
		return -1;
	}
	if (!mysqlConnector.ExcuteSql())
	{
		return -1;
	}
	if (!mysqlConnector.InitBindCol())
	{
		return -1;
	}
	//	读取人物 信息
		int64_t n64accountid = 0;
		int nroleid = 0;
		int nclassid = 0;
		if (!mysqlConnector.BindCol(E_Data_Type_Int64, &n64accountid, sizeof(int64_t)))
		{
			return -1;
		}
		if (!mysqlConnector.BindCol(E_Data_Type_Int, &nroleid, sizeof(int)))
		{
			return -1;
		}
		if (!mysqlConnector.BindCol(E_Data_Type_String, szcharname, 20))
		{
			return -1;
		}
		if (!mysqlConnector.BindCol(E_Data_Type_Int, &nclassid, sizeof(int)))
		{
			return -1;
		}
		while (mysqlConnector.Fetch())
		{
//			char sztranslatedname[21];memset(sztranslatedname,0,sizeof(sztranslatedname));
//			iconv_t conhandle = iconv_open("UTF-8", "GB2312");
//
//			if (conhandle != (iconv_t)-1)
//			{
//				char* p1 = szcharname;
//				char* p2 = sztranslatedname;
//				size_t insize = 20;
//				size_t outsize = 20;
//				size_t sTranslatedsize = iconv(conhandle, &p1, &insize, &p2, &outsize);
//				if (sTranslatedsize == (size_t)-1)
//				{
//					printf("翻译错误");
//				}
//			}
			printf("record info:%lld, %d, %s, %d\n", n64accountid, nroleid, szcharname, nclassid);
		}
	//	读取物品信息
	return 0;
}
int addonerole(XMySqlConnector& mysqlConnector)
{
	char szcharname[21] = "增强电动车";

	if (!mysqlConnector.InitBindParam())
	{
		return -1;
	}
	int64_t n64accid = 3;
	int nroleid=5;
	int classid=1;
	int addresult = 0;

	if (!mysqlConnector.BindParam(E_Data_Type_Int64, &n64accid, sizeof(int64_t), E_Param_Input))
	{
		return -1;
	}
	if (!mysqlConnector.BindParam(E_Data_Type_Int, &nroleid, sizeof(int), E_Param_Input))
	{
		return -1;
	}
	if (!mysqlConnector.BindParam(E_Data_Type_String, szcharname, strlen(szcharname), E_Param_Input))
	{
		return -1;
	}
	if (!mysqlConnector.BindParam(E_Data_Type_Int, &classid, sizeof(int), E_Param_Input))
	{
		return -1;
	}
	if (!mysqlConnector.BindParam(E_Data_Type_Int, &addresult, sizeof(int), E_Param_Output))
	{
		return -1;
	}
	if (!mysqlConnector.SetProcedureName("addrole"))
	{
		return -1;
	}
	if (!mysqlConnector.ExcuteSql())
	{
		char szError[256] = "";
		mysqlConnector.GetError(szError, 255);
		printf("mysqlerror:%s\n", szError);
		return -1;
	}
	if (!mysqlConnector.FreeMoreResult())
	{
		return -1;
	}
	if (!mysqlConnector.GetOutputResult())
	{
		return -1;
	}
	return 0;
}

int main(int nArgc, char* argv[])
{
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGPIPE, &action, NULL);

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGHUP, &action, NULL);


	XMySqlConnector mysqlConnector;

	//mysqlConnector.SetDBServerAddress("127.0.0.1", "test", "root", "wenshengming");


	//mysqlConnector.SetDBServerAddress("127.0.0.1", "account", "root", "wenshengming");
	mysqlConnector.SetDBServerAddress("127.0.0.1", "gameworld", "root", "wenshengming");

	if (!mysqlConnector.Initialize())
	{
		return -1;
	}
	if (!mysqlConnector.Connect())
	{
		return -1;
	}
	mysqlConnector.ExcuteSqlDirect("set NAMES utf8;");
	printf("connect ok!!");

	//	测试调用存储过程并且获得out参数 使用account数据库
//	if (testcallprocedureandreturnout(mysqlConnector) != 0)
//	{
//		return -1;
//	}

	//	测试获取一个结果集并且获取out参数 使用account数据库
//	if (testcallprocedurereadresult(mysqlConnector) != 0)
//	{
//		return -1;
//	}

	//	测试多个结果集和获取out参数
	if (testmultiresultandreturnout(mysqlConnector) != 0)
	{
		return -1;
	}
//	if (!addonerole(mysqlConnector) != 0)
//	{
//		return -1;
//	}
	mysqlConnector.DisConnect();
	return 0;
}


