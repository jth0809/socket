#include<libpq-fe.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
//#include <QJsonParseError>
#include <QJsonObject>
#include<QRandomGenerator>

using namespace std;
class Dbcon {
public:
    Dbcon() {
        conn = PQconnectdb(conninfo);
        if (PQstatus(conn) != CONNECTION_OK) {
            PQfinish(conn);
            exit(1);
        }
    }

    ~Dbcon() {
        PQfinish(conn);
    }

    void insertPrivateChat(const char *from_id, const char *to_id, const char *message){
        const char *value[3] = {from_id, to_id, message};
        if(strlen(message) > 0){
            PGresult *res = PQexecParams(conn,"INSERT INTO private_chat(from_id, to_id, message) VALUES($1, $2, $3)", 3, NULL, value, NULL, NULL, 0);
            PQclear(res);
        }
    }

    void insertFriends(const char *from_id, const char *to_id){
        if(to_id != NULL && to_id != ""){
            const char *value[2] ={from_id, to_id};
            PGresult *res = PQexecParams(conn,"INSERT INTO friends(user_id, friend_id) VALUES($1, $2)", 2, NULL, value, NULL, NULL, 0);
            PQclear(res);
        }
    }
    //내아이디로 조회를 함 -> 상대방 아이디를 추출해서 배열로 만듬 -> 내아이디와 상대방 아이디를 조건으로 다시 조회를 함 -> 채팅방 개설
    //친구추가 해야만 메시지 보낼수 있음 -> 친구추가를 당한 사람이 접속중이어야만 서버에서 메시지를 송신함;
    //개인채팅 클릭시 채팅방 업데이트 함 -> 클라이언트에 db를 내재화? 연동?

    QJsonObject loadPrivateChat(const char *from_id){
        QTextStream(stdout) << "loading start private chat: " << "\n";
        const char *id[1] = {from_id};
        PGresult *res = PQexecParams(conn,"SELECT from_id, to_id FROM private_chat WHERE from_id = $1 UNION SELECT from_id, to_id FROM private_chat WHERE to_id = $1", 1, NULL, id, NULL, NULL, 0);
        int size = PQntuples(res);
        //const char *to_id[size];
        QSet<QString> to_id;
        for(int i=0; i<size; i++){
            QString buf = PQgetvalue(res,i,0);
            QString buf2 = PQgetvalue(res,i,1);
            QTextStream(stdout) << "bufs:"<< buf <<" : "<< buf2 << "\n";
            to_id.insert(buf2);
            to_id.insert(buf);
        }
        PQclear(res);
        QTextStream(stdout) << "loading finish private chat start json: " << "\n";
        qDebug() << to_id;
        QJsonArray jsonArray2D,jsonidArray;
        for(const auto& i : to_id){
            const char *value[2] ={from_id,i.toUtf8().constData()};
            PGresult *res = PQexecParams(conn,"SELECT message FROM private_chat WHERE (from_id = $1 AND to_id = $2) OR (from_id = $2 AND to_id = $1)",2,NULL,value, NULL, NULL, 0);
            QJsonArray jsonArray;
            for (int j = 0; j < PQntuples(res); ++j) {
                if(PQgetvalue(res,j,0) != NULL)
                    jsonArray.append(PQgetvalue(res,j,0));
            }
            if(!jsonArray.isEmpty())
                jsonArray2D.append(jsonArray);
            if(from_id != i && i != "")
                jsonidArray.append(i);
            PQclear(res);
        }
        QTextStream(stdout) << "end json: " << "\n";
        QJsonObject json;
        if(!jsonArray2D.isEmpty())
            json["private_chats"] = jsonArray2D;
        else
            json["private_chats"] = "";
        if(!jsonidArray.isEmpty())
            json["private_ids"] = jsonidArray;
        else
            json["private_ids"] = "";
        json["type"] = "private_load";
        QTextStream(stdout) << json["private_chats"].toString() << "\n";
        return json;

    }

    QJsonArray loadFriends(const char *from_id){
        const char *id[1] = {from_id};
        PGresult *res = PQexecParams(conn,"SELECT friend_id FROM friends WHERE user_id = $1", 1, NULL, id, NULL, NULL, 0);
        int size = PQntuples(res);

        QJsonArray jsonArray;
        for(int i=0; i<size; i++){
            if(strlen(PQgetvalue(res,i,0)) > 0){
                QTextStream(stdout) << "value: "<< PQgetvalue(res,i,0) << "\n";
                jsonArray.append(PQgetvalue(res,i,0));
            }
        }
        PQclear(res);
        return jsonArray;
    }

    const char *login(const char *user_id, const char *user_pwd){
        const char *value[2] = {user_id,user_pwd};

        PGresult *res = PQexecParams(conn, "SELECT id, pwd FROM member WHERE id = $1 AND pwd = $2 AND auth = 0", 2, NULL, value, NULL, NULL, 0);

        if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
            fprintf(stderr, "error query: %s\n", PQresultErrorMessage(res));
            PQclear(res);
            return "0";
        }

        PQclear(res);
        PGresult *res2 =  PQexecParams(conn, "UPDATE member SET auth = 1 WHERE id = $1",1, NULL, value, NULL, NULL, 0);
        PQclear(res2);
        return "1";
    }
    const char *signup(const char* id, const char* pwd){
        if ((id == "" || pwd == "") || ( id == NULL|| pwd == NULL ))
            return "0";
        const char *value[2] = {id, pwd};
        PGresult *res = PQexecParams(conn,"INSERT INTO member(id,pwd) VALUES($1, $2)", 2, NULL, value, NULL, NULL, 0);

        if (PQresultStatus(res) == PGRES_TUPLES_OK || PQresultStatus(res) == PGRES_COMMAND_OK) {
            PQclear(res);
            return "1";
        }
        PQclear(res);
        return "0";
    }
    const char *logout(const char *user_id){
        const char *value[1];
        value[0] = user_id;
        PGresult *res = PQexecParams(conn, "UPDATE member SET auth = 0 WHERE id = $1",1, NULL, value, NULL, NULL, 0);
        PQclear(res);
        return "1";
    }


private:
    const char *conninfo = "host=localhost port=5432 dbname=chatserver user=postgres password=1";

    int auth = 0;

    PGconn *conn;


};

