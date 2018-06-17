// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos
//
// Official Site: www.creativesd.com.br
// Terms of Agreement and Authorship in: http://creativesd.com.br/?p=termos
#ifndef _CREATIVESD_HPP_
#define _CREATIVESD_HPP_

// CreativeSD Emulator Version!
// Do not change, the version is specified by the developers for solutions of bugs or errors.
#ifndef CSDVERSION
	#define CSDVERSION "3.0.00"
#endif

// Packetver
#ifndef PACKETVER
	#define PACKETVER 20180530
#endif

// Packet Obfuscation
//#ifndef PACKET_OBFUSCATION
	//#define PACKET_OBFUSCATION
	//#define PACKET_OBFUSCATION_KEY1 0x37DC4B86
	//#define PACKET_OBFUSCATION_KEY2 0x5EDDF7A5
	//#define PACKET_OBFUSCATION_KEY3 0x08715329
	//#define PACKET_OBFUSCATION_WARN
//#endif

// Queue System
#ifndef QUEUE_SYSTEM_ENABLE
	// Enable Queue System
	// Comment out the '//' line to disable a system.
	#define QUEUE_SYSTEM_ENABLE

	// Is setting should be in accordance with the 'SCRIPT_MAX_ARRAYSIZE' if you increase this setting,
	// variable vectorized scripts will not accept indexing and will miss data in scripting processes.
	#define MAX_QUEUE_PLAYERS	127

	// Queue Name Length
	#define	QUEUE_NAME_LENGTH	30

	// Enter the queue message id.
	// This is for multi language support, so feel free to change it.
	#define QUEUE_MSG_TXT	1750
#endif

// Battleground Warfare
#ifndef BG_WARFARE_ENABLE
	// Enable BG Warfare
	// Comment out the '//' line to disable a system.
	#define BG_WARFARE_ENABLE
	
	// Max Rewards Read
	#define MAX_BG_REWARDS	120

	// Max BG Guild Positions
	#define MAX_BG_POSITION	4

	// Enter the BG message id.
	// This is for multi language support, so feel free to change it.
	#define BG_MSG_TXT 1650
#endif

// Ranked System
#ifndef RANKED_SYSTEM_ENABLE
	// Enable Ranked System
	// Comment out the '//' line to disable a system.
	#define RANKED_SYSTEM_ENABLE

	// Max Ranks DB
	#define	MAX_RANKED_DB	15

	// Max Aura in DB Row
	#define MAX_RANKED_AURA 3

	// Enter the ranked message id.
	// This is for multi language support, so feel free to change it.
	#define RANKED_MSGTXT	1600
#endif

// Stuff Items
#ifndef STUFF_ITEMS_ENABLE
	// Enable Stuff Items
	// Comment out the '//' line to disable a system.
	#define STUFF_ITEMS_ENABLE
#endif

// Organized Shops
#ifndef MAX_OSHOPS_BLOCK
	// Max Vending Block
	#define MAX_OSHOPS_BLOCK	4

	// Enter the Organized Shop message id.
	// This is for multi language support, so feel free to change it.
	#define OSHOP_MSG_TXT 1790
#endif

// Restock System
#ifndef MAX_RESTOCK
	// Max Items in Restock
	#define MAX_RESTOCK 100
#endif

// Bind Clock
#ifndef MAX_BIND_CLOCK
	// Max Bind Block
	#define MAX_BIND_CLOCK 30
#endif

// Presence
#ifndef PRESENCE_MSG_TXT
	// Enter the presence message id.
	// This is for multi language support, so feel free to change it.
	#define PRESENCE_MSG_TXT 1796
#endif

// Arealoot
#ifndef AREALOOT_MSG_TXT
	// Enter the arealoot message id.
	// This is for multi language support, so feel free to change it.
	#define AREALOOT_MSG_TXT 1797
#endif

// Event Room
#ifndef EVENTROOM_MAP
	// Event Room Map
	#define EVENTROOM_MAP "event_room"

	// Event Room x and y coordinate
	#define EVENTROOM_X 99
	#define EVENTROOM_Y 82

	// Enter the event room message id.
	// This is for multi language support, so feel free to change it.
	#define EVENTROOM_MSG_TXT 1799
#endif

#endif /* _CREATIVESD_HPP_ */