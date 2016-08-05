#include "include.h"


//判断是否时json字符串，如果是则返回json_object结构体，否则返回NULL
struct json_object *json_checker(const char *str)
{
    struct json_tokener *tok;
    struct json_object *obj;
    tok = json_tokener_new();
    obj = json_tokener_parse_ex(tok, str, -1);
    if(!obj){
	json_tokener_free(tok);
	return NULL;
    }

    if(tok->err != json_tokener_success)
	obj=(struct json_object *)error_ptr(-tok->err);

    return obj;
}

//解析三张表共有的数据
void json_table_commondata(json_object *fat, int *type, char **owndev,int *result)
{
    struct json_object *json_type, *json_owndev, *json_result;
    json_type = json_object_object_get(fat, "type");
    json_owndev = json_object_object_get(fat, "owndev");
    json_result = json_object_object_get(fat, "result");

    *type = json_object_get_int(json_type);
    *owndev = (char *)json_object_get_string(json_owndev);
    *result = json_object_get_int(json_result);
}


//解析usr_psd的json数据格式
void json_usrpsd_todata(json_object *fat, int *type, char **owndev, char **usr, char **psd, int *status, int *result, int *login)
{
    struct json_object *json_type, *json_owndev, *json_usr, *json_psd;
    struct json_object *json_status, *json_result, *json_login;
    json_type = json_object_object_get(fat, "type");
    json_owndev = json_object_object_get(fat, "owndev");
    json_usr = json_object_object_get(fat, "usr");
    json_psd = json_object_object_get(fat, "password");
    json_status = json_object_object_get(fat, "status");
    json_result = json_object_object_get(fat, "result");
    json_login = json_object_object_get(fat, "login");

    *type = json_object_get_int(json_type);
    *owndev = (char *)json_object_get_string(json_owndev);
    *usr = (char *)json_object_get_string(json_usr);
    *psd = (char *)json_object_get_string(json_psd);
    *status = json_object_get_int(json_status);
    *result = json_object_get_int(json_result);
    *login = json_object_get_int(json_login);
}

//解析env_dev的json数据格式表
void json_envdev_todata(json_object *fat, int *type, char **owndev, char **envdev, int *result, char **tmp, char **hum)
{
    struct json_object *json_type, *json_owndev, *json_envdev, *json_result;
    struct json_object *json_tmp, *json_hum;
    json_type = json_object_object_get(fat, "type");
    json_owndev = json_object_object_get(fat, "owndev");
    json_envdev = json_object_object_get(fat, "envdev");
    json_result = json_object_object_get(fat, "result");
    json_tmp = json_object_object_get(fat, "tmp");
    json_hum = json_object_object_get(fat, "hum");

    *type = json_object_get_int(json_type);
    *owndev = (char *)json_object_get_string(json_owndev);
    *envdev = (char *)json_object_get_string(json_envdev);
    *result = json_object_get_int(json_result);
    *tmp = (char *)json_object_get_string(json_tmp);
    *hum = (char *)json_object_get_string(json_hum);

    printf("i am json_envdev_todata\n");
    printf("type:%d,owndev:%s,envdev:%s\n", *type, *owndev, *envdev);

}

//解析phone_dev的json数据表格式
void json_phonedev_todata(json_object *fat, int *type, char **owndev, char **desctldev, char **desdev, int *status, int *result, int *tmp, int *hum)
{
    struct json_object *json_type,*json_owndev,*json_desctldev, *json_desdev;
    struct json_object *json_status, *json_result,*json_tmp, *json_hum;

    json_type = json_object_object_get(fat, "type");
    json_owndev = json_object_object_get(fat, "owndev");
    json_desctldev = json_object_object_get(fat, "desctldev");
    json_desdev = json_object_object_get(fat, "desdev");
    json_status = json_object_object_get(fat, "status");
    json_result = json_object_object_get(fat, "result");
    json_tmp = json_object_object_get(fat, "tmp");
    json_hum = json_object_object_get(fat, "hum");

    *type = json_object_get_int(json_type);
    *owndev = (char *)json_object_get_string(json_owndev);
    *desctldev = (char *)json_object_get_string(json_desctldev);
    *desdev = (char *)json_object_get_string(json_desdev);
    *status = json_object_get_int(json_status);
    *result = json_object_get_int(json_result);
    *tmp = json_object_get_int(json_tmp);
    *hum = json_object_get_int(json_hum);

}



