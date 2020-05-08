// IcyTower13TrainerClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "IcyTowerConsts.h"

using namespace boost::interprocess;
using namespace std;
void show_menu() {
	cout << "Waiting for next command: ([msgtype] [feature] [[value]])" << endl;
	cout << "\t:: Message Types::" << endl;
	cout << "1 --- ChangeEnabledFeature	" << endl;
	cout << "2 --- ChangeValueFeature" << endl;	
	cout << "3 --- ChangeEnableAll" << endl;
	
	cout << "\t:: Features::" << endl;
	cout << "1 --- FreezeFloor" << endl;
	cout << "2 --- FreezeCurrentCombo" << endl;
	cout << "3 --- BestCombo" << endl;
	cout << "4 --- AdditionalScore" << endl;
}
int main()
{
	message_queue mq(open_only, MSGQUEUE_NAME);
	while (true) {
		show_menu();
		int msgtype = 0, feature = 0, value = 0;
		scanf_s("%d %d %d", &msgtype, &feature, &value);
		//construct message
		IcyTowerMessage msg(static_cast<IcyTowerMessageType>(msgtype), static_cast<IcyTowerFeature>(feature), value);
		mq.send(&msg, sizeof(msg), 0);
		cout << "Sent message!" << endl;
	}
}

