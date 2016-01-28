/**
	This thrift files explains the differet services (outside of UDP keep alive) Mercury
	will provide
*/

struct Message {
	1. i64 source,			//UserID number of the sender
	2. i64 destination, //UserID number of the message destination. 
											//<=0 is an ignore or is destined for Mercury
	3. i64 id						//Throw away unique ID. Probably will be Epoch timestamp
	4. string data,			//JSON send out
}

service Mercury {
	void ping(),
	void HelloWorld(1:Message message), //Sends hello in data
} 

