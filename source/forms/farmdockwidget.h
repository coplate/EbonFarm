#ifndef FARMDOCKWIDGET_H
#define FARMDOCKWIDGET_H

#include <QDockWidget>

#include <iostream>
class WhiteBoard;

namespace Ui {
class FarmDockWidget;
}

class FarmDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit FarmDockWidget(QWidget *parent = 0);
    ~FarmDockWidget();

    bool initialize(WhiteBoard *wb);
    void send_char(const QString &message);
private:
    Ui::FarmDockWidget *ui;
    WhiteBoard *whiteBoard;

    //void FSM(const char *caller);
    void fail_abort(std::string reason);
    void fail_stop(std::string reason);
    void farm(const QString current_weapon, const QString &attack_command, int rounds);
    void  getMessage();
    bool  eat();
    bool  loot(const QString &itemClass=QString("?"), bool farm=false);
    bool  offer();
    void  refill();
    bool send_and_expect(const QString &send, const QString &expect);
    void pb_start();
signals:
    //void change_state(int old_state, int new_state);
    void change_state();
private slots:
   void on_pushButton_clicked();
   void timer_abort();
   void on_stopButton_clicked();

   void on_pushButton_2_clicked();

   void on_pushButton_3_clicked();

   void on_checkBox_toggled(bool checked);

   void on_manualFarmButton_clicked();

public slots:
   void alert_tiles_finished();
    void alert_changed_state(int old_state, int new_state);
};

#endif // FARMDOCKWIDGET_H
