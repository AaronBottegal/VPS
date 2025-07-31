#ifndef VPS_H
#define VPS_H

#include <QAbstractSocket>
#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class VPS;
}
QT_END_NAMESPACE

class VPS : public QMainWindow
{
    Q_OBJECT

public:
    VPS(QWidget *parent = nullptr);
    ~VPS();
    //Our internal crap.
    void OBS_Create_New_Scene();

public slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError err);
    void msgrecv(const QString &msg);
    void framerecv(const QString &msg, bool fin);
    void process_websock_data(QJsonDocument &doc);

private slots:
    void on_BTN_Connect_clicked();

    void on_BTN_SCENE1_clicked();

    void on_BTN_SCENE1_2_clicked();

    void on_BTN_SCENE1_3_clicked();

private:
    Ui::VPS *ui;
};

extern QString NEW_SCENE_ID;

#endif // VPS_H
