/*Copyright 2014 Matthew Carlson

This file is part of EbonFarm, an extension of EbonHack.

    EbonFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EbonFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EbonFarm.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "farmdockwidget.h"
#include "ui_farmdockwidget.h"
#include <QMessageBox>

#include <MainWindow.hpp>

#include <WhiteBoard.hpp>
#include <TelnetProtocol.hpp>
using namespace std;

FarmDockWidget::FarmDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::FarmDockWidget)
{
    ui->setupUi(this);

}

FarmDockWidget::~FarmDockWidget()
{
    delete ui;
}
QString message="";
QString status="";

QString farmRubbish;
bool expecting_alerts = false;
long expected_alerts = 0;
QTimer *timer;
int attacks = 0;
QString splitWeapon;
QString killWeapon;
QString splitMessage;
QString killMessage;
QString empty = "";

QString wield_command="w";
QString pause_command = ".";

QString pick_up_command = ",";
QString enter_command = "\n";
QString esc_command = "\x1B";
QString pick_up_what_response = "Pick up what?";
QString direction;
QString rev_direction;
uint8_t playerline;
uint8_t playerpos;
uint8_t farmline;
uint8_t farmpos;
int state=0;
int aborted = 0;
bool running = false;
QMap<QString, QPair<int, int> > directions;
QMap<QString, QString> rev_dirs;


void FarmDockWidget::fail_abort(std::string reason)
{


    fail_stop("abort: "+reason);

    if( aborted == 0 ){
        aborted ++;

        QMessageBox msgBox;
        msgBox.setText("Please fix the problem.");
        //msgBox.setStandardButtons(QMessageBox::OK);
        //msgBox.setDefaultButton(QMessageBox::OK);
        msgBox.exec();
    }
}
void FarmDockWidget::fail_stop(std::string reason)
{

    cout<<"Player is at position "<<int(playerline)<<":"<<int(playerpos)<<endl;
    cout<<"Player symbol is "<<this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(playerpos, playerline)<<endl;
    cout<<"Farm is at position "<<int(farmline)<<":"<<int(farmpos)<<endl;
    cout<<"Farm symbol is "<<this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(farmpos, farmline)<<endl;

    cout<<"Message is ";
    for(int i = 0; i < this->whiteBoard->getTelnetPro()->getTelnetWindow()->getWidth(); i++){
        cout<<this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(i, 0);
    }
    cout<<endl;

    running = false;
    state = 0;
    cout<<"Finished Farming: "<<reason<<endl;
    timer->stop();
    expecting_alerts= false;
    farmRubbish.clear();
    this->ui->groupBox_7->setEnabled(true);
    this->ui->pushButton->setEnabled(true);


}
void  FarmDockWidget::refill(){
    QString m2 = "";
    message.clear();
    status.clear();
    for(int i = 0; i < this->whiteBoard->getTelnetPro()->getTelnetWindow()->getWidth(); i++){
        message.append(this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(i, 0));
        m2.append(this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(i, 1));
        status.append(this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(i, 23));
    }
    message.append(m2.trimmed());
    cout<<"Message is "<<message.toStdString()<<endl;
    cout<<"Status is "<<status.toStdString()<<endl;

}

bool  FarmDockWidget::eat(){

    QString command ="";
    try{
        cout<<"I need to eat"<<endl;
        if(status.contains("atiated") || message.contains("atiated")){
            cout<<"Satioated, do not eat"<<endl;
            return true;
        }
        if( ! running ){return false;}
        if( message.contains("feel hungry.--More--") ){
            cout<<"Skipping --More-- prompt"<<endl;
            send_and_expect(enter_command, empty);
        }

        send_and_expect("e", empty);
        if( message.contains("do you want to eat? [elw or ?*]") ){
            cout<<"proper food"<<endl;
        }else{
            cout<<"!!== Food at 'e'; Lizard at 'l'; Wolfsbane at 'w'; NO OTHER FOOD ==!! "<<endl;
            return false;
        }

        send_and_expect("e", empty);
        if( message.contains("goes dark") ){
            cout<<"The world sun and went dark"<<endl;
            return false;
        }else if( message.contains("light headed") ){
            cout<<"confused"<<endl;
            return false;
        }else if(status.contains("Hungry") || message.contains("feel hungry")){
            cout<<"You are still hungry"<<endl;
            return false;
        }else if(status.contains("atiated") || message.contains("atiated")){
            cout<<"Satioated, eating food rations wont do that"<<endl;
            return false;

        }
    }
    catch(int e){
        cout<<"Exception is"<<e<<endl;
        return false;
    }

    return true;

}

bool  FarmDockWidget::loot(const QString &itemClass){

    bool items_found = true;
    QString command ="";

    try{


        while(( items_found == true ) && ( strchr(")[!?/=+*(`$%0_\"",this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(farmpos, farmline))!= NULL) ) {
            items_found = false;
            cout<<"I need to pick up scrolls of scare monster, I might as well pick them all up"<<endl;
            if( ! running ){
                return false;
            }

            send_and_expect("m"+direction, empty);

            if( this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorY() != farmline){
                cout<<"cursor is not at farmline"<<endl;
                return false;
            }
            if( this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorX() != farmpos){
                cout<<"cursor is not at farmpos"<<endl;
                return false;
            }
            if(this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(farmpos, farmline) != '@'){
                cout<<"player is not on farmspot"<<endl;
                return false;
            }
            if( ! running ){return false;}

            send_and_expect(pick_up_command, pick_up_what_response);
            send_and_expect(itemClass, pick_up_what_response);
            send_and_expect(enter_command, empty);


            if( message.contains(" - ") ){
               items_found = true;
            }else if( message.contains("little trouble") ){
                items_found = true;
                send_and_expect("y", empty);
            }
            if( items_found == 1){
                while( message.contains("--More--") ){
                    send_and_expect(esc_command, empty);
                    if( message.contains("little trouble")){
                        send_and_expect("y", empty);
                    }else
                    if( message.contains("much trouble")){
                        send_and_expect("y", empty);
                    }else
                    if( message.contains("extreme difficulty")){
                        send_and_expect("y", empty);
                    }else{
                        send_and_expect(esc_command, empty);
                        break;
                    }
                }
                if( ! itemClass.contains("?")){
                    send_and_expect("D", "Drop what type of");
                    send_and_expect(itemClass, "Drop what type of");
                    send_and_expect(enter_command, "What would you like to drop");
                    send_and_expect(itemClass, "What would you like to drop");
                    send_and_expect(enter_command, empty);
                    send_and_expect(esc_command, empty);

                    send_and_expect(pick_up_command, pick_up_what_response);
                    send_and_expect(itemClass, pick_up_what_response);
                    send_and_expect(enter_command, empty);
                    if( message.contains("little trouble") ){
                        send_and_expect("y", empty);
                    }
                    while( message.contains("--More--") ){
                        send_and_expect(esc_command, empty);
                        if( message.contains("little trouble")){
                            send_and_expect("y", empty);
                        }else
                        if( message.contains("much trouble")){
                            send_and_expect("y", empty);
                        }else
                        {
                            send_and_expect(esc_command, empty);
                            break;
                        }
                    }
                }
            }

            if( message.contains("--More--")){
                send_and_expect(esc_command, empty);
            }



            send_and_expect( "m"+rev_direction, "burned into the floor");

            if( this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorY() != playerline){
                cout<<"cursor is not at farmline"<<endl;
                return false;
            }
            if( this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorX() != playerpos){
                cout<<"cursor is not at farmpos"<<endl;
                return false;
            }
            if(this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(playerpos, playerline) != '@'){
                cout<<"player is not on SafeSpot"<<endl;
                return false;
            }
            if( ! running ){return false;}

            if( items_found ){

                send_and_expect( "l", "uncursed sack here, loot it? [ynq");
                send_and_expect( "y", "You carefully open the sack...--More-");
                send_and_expect( enter_command, "Do what?");
                send_and_expect( "i", "Put in what type of objects?");
                send_and_expect( itemClass, "Put in what type of objects?");
                send_and_expect( enter_command, "Put in what?");
                send_and_expect( ".", "Put in what?");
                send_and_expect( enter_command, "You put ");

                command = "\x1B";
                send_char(command);
                refill();
                if( message.trimmed().length() > 0 ){
                    cout<<"Normally there should me no message after puting scrolls in"<<endl;
                    return false;
                }
            }
        }
    }
    catch(int e){
        cout<<"Exception is"<<e<<endl;
        return false;
    }

    return true;

}
bool FarmDockWidget::send_and_expect(const QString &send, const QString &expect){
    if( ! running ){throw 1;}
    send_char(send);
    refill();
    if( expect.length() > 0){
        if( ! message.contains(expect)){
            cout<<"REsponse to command did not contain expected result"<<endl;
            cout<<"sent >"<<send.toStdString()<<"< received \n>"<<message.toStdString()<<"< expected \n>"<<expect.toStdString()<<"<"<<endl;
            running = false;
            throw 4;
            //return false;
        }
    }
    if( ! running ){throw 1;}
    return true;
}

bool  FarmDockWidget::offer(){

    QRegExp rx("There (are|is) ([\\d]+|an?) (.*) corpse[s]? here; sacrifice (one|it)[?]");

    int sacrifice = 0;
    QString command ="";
    try{
    cout<<"LEts Sacrifice everything, home im wearing gloves!"<<endl;
    if( ! running ){
        return false;
    }
    send_and_expect("m"+direction, empty);
    // check player is at farm

    if( this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorY() != farmline){
        cout<<"cursor is not at farmline"<<endl;
        return false;
    }
    if( this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorX() != farmpos){
        cout<<"cursor is not at farmpos"<<endl;
        return false;
    }
    if(this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(farmpos, farmline) != '@'){
        cout<<"player is not on farmspot"<<endl;
        return false;
    }
    sacrifice = 1;
    while( sacrifice > 0){
        send_and_expect("#offer\n", empty);

        if(rx.indexIn(message)>=0 ){
            cout<< rx.cap(1).toStdString()<<endl;
            cout<< rx.cap(2).toStdString()<<endl;
            cout<< rx.cap(3).toStdString()<<endl;
        }else{
            cout<<"No corpses to sacrifice"<<endl;
            if(message.contains("What do you want to sacrifice?")){
                send_and_expect(enter_command, empty);
                if(!message.contains("Never mind")){
                    fail_abort("Sacrifice error expecting nevermind");
                }
            }
            sacrifice = 0;
            break;
        }
        if(! rx.cap(2).contains("a")){
            sacrifice = rx.cap(2).toInt();
        }else{
            sacrifice = 1;
        }
        send_and_expect("y", empty);
        //;Consumed in a flash
        if(!message.contains("Your sacrifice is consumed in a flash of light!")){
            cout<<"sacrifice not consumed in a flash"<<endl;
            return false;
        }
        if(message.contains("--More--")){
            send_and_expect(enter_command, empty);
        }


        if(message.contains("hopeful feeling")){
            cout<<"sacrifice consumed and hopeful"<<endl;

        }
        if(
                message.contains("Your sacrifice is consumed in a flash of light!") ||
                message.contains("reconciliation") ||
                message.contains("four-leaf clover") ||
                message.trimmed().length() == 0
                ){

            send_and_expect("#pray\n", empty);
            if( !message.contains("Are you sure you want to pray?")){
                cout<<"Prayer message was not correct;"<<endl;
            }

            send_and_expect("y", empty);
            if( !message.contains("You begin praying to --More--" ) || !message.contains("--More--")){
                cout<<"Prayer response was not correct;"<<endl;
            }
            for( int i = 0; i < 16; i++){
                send_and_expect(enter_command, empty);
                if( !message.contains("--More--")){
                    break;
                }
            }

        }else{
            cout<<"Not safe to pray, some unknown message"<<endl;

        }
    }
    send_and_expect("m"+rev_direction, empty);
    if(!message.contains("burned into the floor")){
        cout<<"cursor is not at farmline"<<endl;
        return false;
    }
    if(message.contains("--More--")){
        send_and_expect(enter_command, empty);
    }
    if( this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorY() != playerline){
        cout<<"cursor is not at farmline"<<endl;
        return false;
    }
    if( this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorX() != playerpos){
        cout<<"cursor is not at farmpos"<<endl;
        return false;
    }
    if(this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(playerpos, playerline) != '@'){
        cout<<"player is not on SafeSpot"<<endl;
        return false;
    }

    }catch(int e){
        cout<<"Exception: "<<e<<endl;
        return false;
    }

    return true;

}
void FarmDockWidget::pb_start(){

    playerline = this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorY();
    playerpos = this->whiteBoard->getTelnetPro()->getTelnetWindow()->getCursorX();

    QList<QRadioButton *> allRadioButtons = ui->groupBox->findChildren<QRadioButton *>();
    foreach (QRadioButton *rb,allRadioButtons)
    {
        if(rb->isChecked())
        {
            direction = rb->text();
            break;
        }
    }

    QPair<int, int> offset = directions.value(direction);
    rev_direction = rev_dirs.value(direction);
    //attackMessage = "F" + direction;
    attacks = this->ui->spinBox_split_attacks->value();
    splitMessage.clear();
    for( int i = 0; i < attacks; i++){
        splitMessage.append("F" + direction);
    }
    killMessage.clear();
    attacks = this->ui->spinBox_kill_attacks->value();
    for( int i = 0; i < attacks; i++){
        killMessage.append("F" + direction);
    }
    //attackMessage += (char)1;// 1 is "ctrl+A" or 'Again'
    farmline = playerline + offset.second;
    farmpos = playerpos + offset.first;



    cout<<"Player is at position "<<int(playerline)<<":"<<int(playerpos)<<endl;
    cout<<"Player symbol is "<<this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(playerpos, playerline)<<endl;
    cout<<"Farm is at position "<<int(farmline)<<":"<<int(farmpos)<<endl;
    cout<<"Farm symbol is "<<this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(farmpos, farmline)<<endl;

    running = true;

}

void FarmDockWidget::farm(const QString current_weapon, const QString &attack_command, int rounds){

    int skip_max = 7;
    int skipped = 0;


    try{
        send_and_expect(wield_command, empty);
        /*
         * if(! message.contains("want to wield")){
            cout<<"Bad response to wield"<<endl;
            return fail_abort();
            break;
        }
        */
        send_char(current_weapon);
        refill();

        for( int i = 0; (i < rounds) && running; i++){
            refill();
            uint8_t altar = this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(farmpos, farmline);
            if(status.contains("Hungry") || message.contains("feel hungry")){
                bool stat = eat();
                if( stat == false)
                {
                    return fail_abort("Eating failed");
                }

            }else if(strchr(")[!?/=+*(`$%0_\"",altar) != NULL ){
                cout<<"Altar has an item on top"<<endl;
                /* altar has an item on it, not a monster, and not food */
                skipped++;
                if( ui->checkBox->isChecked() ){
                    cout<<"Skipped is"<<skipped<<endl;
                    if( skipped > 0){
                        bool stat = offer();
                        if( stat == false)
                        {
                            return fail_abort("Offering failed");
                        }

                    }
                }
                /* make sure altar is still empty, or figure out how to loot wile I am still on it */
                altar = this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(farmpos, farmline);
                if(strchr(")[!?/=+*(`$%0_\"",altar) != NULL ){
                    cout<<"Altar STILL has an item on top"<<endl;
                    if( skipped > skip_max){
                        bool stat = loot();
                        if( stat == false)
                        {
                            return fail_abort("looting failed");
                        }
                        skipped = 0;
                    }
                }
                send_char(pause_command);
            }else if(strchr("c@",altar) != NULL ){
                cout<<"Altar has a dangerous monster"<<endl;
                return fail_abort("Altar has a dangerous monster");
            }else if(strchr("PsSafZB:",altar) != NULL ){
                cout<<"Altar has a safe monster on top"<<endl;
                /* the 'safe monsters I am willing to autofarm */
                skipped = 0;

                farmRubbish.clear();
                if( attack_command.length() > 0){
                    send_char(attack_command);
                    cout<<"Farm Rubbish: "<<farmRubbish.toStdString()<<endl;

                }
            }else{
                if( skipped > skip_max){
                    cout<<"The farmline does not have a pudding on it";
                    return fail_stop("skipped > skip_max");
                }else{
                    send_char(pause_command);
                    skipped++;
                }

            }
        }
    }
    catch(int e){
        cout<<"Exception is"<<e<<endl;
        return;
    }

}

void FarmDockWidget::on_pushButton_clicked()
{
    int splitAttacks = ui->spinBox_split_rounds->value();
    int killAttacks = ui->spinBox_kill_rounds->value();
    killMessage = "";
    splitMessage = "";
    splitWeapon = this->ui->lineEdit_split->text();
    killWeapon = this->ui->lineEdit_kill->text();

    pb_start();

    cout<<"Farming with splitWeapon: "<<splitWeapon.toStdString()<<" and killWEapon: "<<killWeapon.toStdString()<<endl;
    cout<<"Will run commands: >"<<splitMessage.toStdString()<<"< "<<endl;
    cout<<"Will run commands: >"<<killMessage.toStdString()<<"< "<<endl;


    splitWeapon = splitWeapon.trimmed();
    killWeapon = killWeapon.trimmed();

    cout<<"Starting: expecting signals"<<endl;




    state = 0;

    refill();

    if( splitAttacks <= 0 || killAttacks <= 0){
        return;
    }
    if( ui->checkBox_split->isChecked() || ui->checkBox_kill->isChecked()){


        while(running){
            if( ui->checkBox_split->isChecked()){
                farm(splitWeapon, splitMessage, splitAttacks);
            }
            if( ui->checkBox_kill->isChecked()){
                farm(killWeapon, killMessage, killAttacks);
            }
        }
    }



}

void FarmDockWidget::send_char(const QString &message)
{

     if( ! running ){throw 1;}
    expecting_alerts = true;
    timer->start(1000);
    expected_alerts = message.length();
    cout<<"Sending message of length: "<< message.length()<<endl;
    cout<<message.toStdString()<<endl;

    this->ui->groupBox_7->setEnabled(false);
    this->ui->pushButton->setEnabled(false);

    QEventLoop loop;
    connect(this, SIGNAL(change_state()), &loop, SLOT(quit()));
    QTimer loop_timer (this);
    connect(&loop_timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    loop_timer.start(4000);
    this->whiteBoard->getTelnetPro()->sendCommand(message);
    loop.exec();
    loop_timer.stop();
    expecting_alerts = false;
    if( running ){
        if(expected_alerts > 0 ){
            cout<<"sendCommand timed out in 4 seconds?"<<endl;
            cout<<"expected alerts:"<<expected_alerts<<endl;
            cout<<"expected alerts:"<<loop_timer.remainingTime()<<endl;
            fail_abort("running set to fals, but still some expected allert");
        }
        this->ui->groupBox_7->setEnabled(true);
        this->ui->pushButton->setEnabled(true);
    }else{
        //stopped it with the stop button
    }

}
bool FarmDockWidget::initialize(WhiteBoard *wb){
    this->whiteBoard = wb;


    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_abort()));

    directions.insert("7", QPair<int, int>(-1,-1));
    directions.insert("4", QPair<int, int>(-1,0));
    directions.insert("1", QPair<int, int>(-1,1));
    directions.insert("8", QPair<int, int>( 0,-1));
    directions.insert("2", QPair<int, int>( 0,1));
    directions.insert("9", QPair<int, int>( 1,-1));
    directions.insert("6", QPair<int, int>( 1,0));
    directions.insert("3", QPair<int, int>( 1,1));

    rev_dirs.insert("7", "3");
    rev_dirs.insert("4", "6");
    rev_dirs.insert("1", "9");
    rev_dirs.insert("8", "2");
    rev_dirs.insert("2", "8");
    rev_dirs.insert("9", "1");
    rev_dirs.insert("6", "4");
    rev_dirs.insert("3", "7");
    return true;
}

void FarmDockWidget::timer_abort()
{
    if( expecting_alerts ){
        this->ui->groupBox_7->setEnabled(false);
        this->ui->pushButton->setEnabled(false);
    }else{
        this->ui->groupBox_7->setEnabled(true);
        this->ui->pushButton->setEnabled(true);

    }
}

void FarmDockWidget::alert_changed_state(int old_state, int new_state)
{

    cout<<"Received CS from"<<old_state<<" to "<<new_state<<endl;

}
bool started=false;
void FarmDockWidget::alert_tiles_finished()
{
    //cout<<"alert_tiles_finished: "<<expecting_alerts<<endl;
    if( expecting_alerts ){
        expected_alerts--;
        //cout<<"expecting More signals"<<expected_alerts<<endl;
        farmRubbish.append(this->whiteBoard->getTelnetPro()->getTelnetWindow()->getByte(farmpos, farmline));
        if( expected_alerts == 0){
            cout<<"Received all my expected alerts"<<endl;

            emit change_state();
        }
    }else{
        if( started == false ){
            this->ui->groupBox_7->setEnabled(true);
            started = true;
        }
    }
}

void FarmDockWidget::on_stopButton_clicked()
{
    running = false;
    state = 0;
    fail_stop("stop clicked");
    //this->FSM("alert_tiles_finished");
}



void FarmDockWidget::on_pushButton_2_clicked()
{
    pb_start();
    cout<<"clicked : "<<ui->comboBox->currentText().toStdString()<<endl;
    QString current_text = ui->comboBox->currentText();
    loot(current_text);
}
