#include "UDPserver.h"
#include "sha2.h"

#include<zdb/zdb.h>

ConnectionPool_T dbpool;
URL_T dburl;

int dbpool_init()
{
    dburl = URL_new("mysql://localhost/udptest?user=udptest&password=Nwv4PGcXrrfdCcnj-");  
    if(dburl==NULL)  
    {  
        printf("URL parse ERROR!\n");  
        return -1;  
    }  
    dbpool = ConnectionPool_new(dburl);
    //设置最大连接数目  
    ConnectionPool_setMaxConnections(dbpool,100);
    //开启线程池  
    ConnectionPool_start(dbpool);
    return 0;
    
}

int dbpool_des()
{
    //将连接池与数据库分离  
    ConnectionPool_stop(dbpool);  
    ConnectionPool_free(&dbpool);    
    URL_free(&dburl);   
    return 0;
}


int is_nosec(char *str) //只允许字母和数字
{
    int length = strlen(str);
    for(int i=0;i<length;i++)
    {
        if( !( (str[i]>='0'&&str[i]<='9') || (str[i]>='a' && str[i]<='z') || (str[i]>='A' && str[i]<='Z') ) ){
            return 1;
        }
    }
    return 0;
}

void messag_handle(int fd, char mbuf[])
{
    //parse
    char seg[] = ",";
    char charlist[6][70]={""};
    int listsum =0;
    char *substr= strtok(mbuf, seg);
    while (substr != NULL) {
        if(strlen(substr)>=70 || is_nosec(substr) ){
            strcpy(mbuf,"");
            return;
        }
        strcpy(charlist[listsum],substr);
        listsum++;
        substr = strtok(NULL,seg);
    }

    Connection_T con;
    ResultSet_T result,result2;
    char query[128];

    if(strcmp(charlist[0],"GET")==0 && listsum==4) {
        
        con = ConnectionPool_getConnection(dbpool);
        sprintf(query, "select pwd,cardpwd_numtop-cardpwd_num,ID,activation from thth_admin where userid = '%s'", charlist[1] );
        result = Connection_executeQuery(con, query);

        if(ResultSet_next(result)==0){
            strcpy(mbuf,"NO-1"); //用户名错误
            Connection_close(con);
            return;
        }

        if(strcmp(ResultSet_getString(result,1), charlist[2]) == 0){
            strcpy(mbuf,"OK-1");

            //检查是否被禁用
            if( ResultSet_getInt(result,4)==0 ){
                strcpy(mbuf,"NO-4"); //用户被禁用
                Connection_close(con);
                return;
            }
            

            //记录本次查询结果
            char nowtime[20]={0};
            time_t t;
            struct tm * lt;
            time (&t);//获取Unix时间戳。
            lt = localtime (&t);//转为时间结构。
            sprintf( nowtime,"%d-%02d-%02d %02d:%02d:%02d",lt->tm_year+1900, (lt->tm_mon+1)%12, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

            int now_times = ResultSet_getInt(result,2);
            int userid = ResultSet_getInt(result,3);

            //查看是否查询过
            sprintf(query, "select card_pwd from thth_works where card_id = '%s' and userid = '%d'", charlist[3], userid);
            result2 = Connection_executeQuery(con, query);
            

            if(ResultSet_next(result2)==0){  //没有查过

                if(now_times<=0){
                    strcpy(mbuf,"NO-3"); //次数用完了
                    Connection_close(con);
                    return;
                }

                sprintf(query, "UPDATE thth_admin SET cardpwd_num = cardpwd_num+1 WHERE userid = '%s'", charlist[1] );  //times减1
                Connection_execute(con, query);

                strcat(mbuf, ",");
                char times[125];
                char devkey[125];
                sprintf(times, "%d", now_times-1 );
                strcat(mbuf, times);
                strcat(mbuf, ",");

                //加密 输入 charlist[3] 输出 output
                //sha224
                unsigned char digest[SHA224_DIGEST_SIZE];
                sha224( (const unsigned char *)charlist[3], strlen(charlist[3]), digest);
                char output[2 * SHA224_DIGEST_SIZE + 1];
                int i;
                output[2 * SHA224_DIGEST_SIZE] = '\0';
                for (i = 0; i < (int) SHA224_DIGEST_SIZE ; i++) {
                    sprintf(output + 2 * i, "%02x", digest[i]);
                }
                //sha224 end
                
                sprintf(query, "insert into thth_works(userid, card_id, card_pwd, search_times) values('%d', '%s', '%s', '1')", userid, charlist[3], output );  //times+1
                Connection_execute(con, query);

                sprintf(devkey, "%s", output );
                strcat(mbuf, devkey);
                
            }else{
                strcat(mbuf, ",");
                char times[125];
                char devkey[125];
                sprintf(times, "%d", now_times );
                strcat(mbuf, times);
                strcat(mbuf, ",");
                sprintf(devkey, "%s", ResultSet_getString(result2,1) );
                strcat(mbuf, devkey);
                sprintf(query, "UPDATE thth_works SET search_times = search_times+1 , update_time = '%s' WHERE userid = '%d' and card_id ='%s'", nowtime, userid , charlist[3]);  //times+1
                Connection_execute(con, query);
            }
        }else{
            strcpy(mbuf,"NO-2"); //密码错误
        }
        Connection_clear(con);
        Connection_close(con);
        return;
    }else {
        strcpy(mbuf,"");
        return;
    }

}

int main(int argc, char const *argv[])
{
    dbpool_init();
    UDPserver_run(100, 200, messag_handle );  //线程数，线程池队列数，消息处理函数
    dbpool_des();
    return 0;
}
