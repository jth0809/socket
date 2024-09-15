#include "dbcon.h"

class ChatServer : public QTcpServer {
    Q_OBJECT
public:
    ChatServer(QObject* parent = nullptr) : QTcpServer(parent) {
        listen(QHostAddress::Any, 12345);
    }
    Dbcon db;
protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket* socket = new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);
        clients.append(socket);
        QTextStream(stdout) << "New connection from " << socket->peerAddress().toString() << "\n";

        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            QByteArray message = socket->readAll();
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(message,&parseError);
            QJsonObject obj = doc.object();
            QJsonObject json;
            QByteArray id, pwd;

            user_id = obj["user_id"].toString().toUtf8();
            privateClients[user_id] = socket;
            //const char *auth;

            if (user_id == ""){
                id = obj["id"].toString().toUtf8();
                pwd = obj["password"].toString().toUtf8();
                obj["auth"] = db.login(id.data(), pwd.data());

                json = credentialProcessing(obj);
                send(socket,json);
            }

            if (user_id != ""){
                connectionProcessing(socket,obj);
            }

        });

        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            QTextStream(stdout) << "disconnected " << "\n";
            db.logout(user_id.data());
            clients.removeAll(socket);
            randomQueue.removeAll(socket);
            randomClients.remove(socket);
            socket->deleteLater();
        });
    }

    void connectionProcessing(QTcpSocket* socket, QJsonObject obj){
        QJsonObject json;
        QByteArray chat = obj["message"].toString().toUtf8();
        user_id = obj["user_id"].toString().toUtf8();
        QByteArray to_id = obj["friend_id"].toString().toUtf8();

        int typeNumber = type[obj["type"].toString()];
        QTcpSocket* client;
        switch(typeNumber){
            case 1:
                for (QTcpSocket* client : clients) {
                    if (client != socket) {
                        send(client,obj);
                    }
                }
                break;
            case 2:
                db.insertPrivateChat(user_id, to_id, chat);
                if (privateClients.contains(to_id)){
                    QTcpSocket* client = privateClients[to_id];
                    send(client,obj);
                }
                break;
            case 3:
                json = db.loadPrivateChat(user_id);
                send(socket,json);
                break;
            case 4:
                db.insertFriends(user_id, to_id);
                break;
            case 5:
                obj["friends"] = db.loadFriends(user_id);
                send(socket,obj);
                break;
            case 6:
                if (randomClients.contains(socket)) {
                    QTcpSocket* client = randomClients[socket];
                    send(client,obj);
                }
                break;
            case 7:
                if(!randomQueue.contains(socket))
                    randomQueue.append(socket);
                QTextStream(stdout) << "random socket connected: " << obj["type"].toString() << "\n";
                obj["message"] = "매칭중";
                obj["type"] = "random_message";
                send(socket,obj);
                if (randomQueue.size() >= 2 && randomQueue.size()%2 == 0){
                    QTcpSocket* first = randomQueue.takeFirst();
                    QTcpSocket* second = randomQueue.takeFirst();
                    randomClients[first] = second;
                    randomClients[second] = first;
                    randomQueue.removeAll(first);
                    randomQueue.removeAll(second);
                    obj["message"] = "매칭 성공";
                    send(first,obj);
                    send(second,obj);
                }
                break;
            case 8:
                if(randomClients.contains(socket)){
                    client = randomClients[socket];
                    obj["message"] = "상대방이 떠났습니다.";
                    send(client, obj);
                    randomClients.remove(randomClients[socket]);
                    randomClients.remove(socket);
                }
                if(randomQueue.contains(socket))
                    randomQueue.removeAll(socket);
                break;
            case 9:
                obj["auth"] = db.logout(user_id.data());
                send(socket,obj);
                break;

        }
    }
    void send(QTcpSocket* socket, QJsonObject json){
        QJsonDocument doc(json);
        QByteArray data = doc.toJson(QJsonDocument::Compact);
        QTextStream(stdout) << "send data: " << data << "\n";
        socket->write(data);
        socket->flush();
    }


    QJsonObject credentialProcessing(QJsonObject obj){
        QJsonObject json;
        QByteArray id = obj["id"].toString().toUtf8();
        QByteArray pwd = obj["password"].toString().toUtf8();

        json["auth"] = obj["auth"];
        json["type"] = obj["type"];

        if (obj["auth"] == "1")
            json["id"] = obj["id"];
        else if (obj["auth"] == "0" && obj["type"] == "signup"){
            json["auth"] = db.signup(id.data(), pwd.data());
        }

        return json;
    }

private:
    QByteArray user_id;
    QList<QTcpSocket*> clients;
    QMap<QByteArray,QTcpSocket*> privateClients;
    QMap<QTcpSocket*,QTcpSocket*> randomClients;
    QList<QTcpSocket*> randomQueue;

    QMap<QString,int> type = { {"public_message",1},
                               {"private_message",2},
                               {"private_load",3},
                               {"friend_insert",4},
                               {"friend_load",5},
                               {"random_message",6},
                               {"random_connection",7},
                               {"random_exit",8},
                               {"logout",9}};
};

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);
    ChatServer server;
    return a.exec();
}

#include "main.moc"
