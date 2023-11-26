#include <iostream>
#include <poll.h>
#include "modules/xcbManager.h"
#include <memory>
#include "modules/SocketUnix.h"
#include "modules/textUtil.h"

int main()
{
    std::cout << "Hello, world!" << std::endl;
    // Je crée le client
    std::unique_ptr<SocketUnix> microWMSocketClient = std::make_unique<SocketUnix>("/tmp/microWM.sock",false);
    microWMSocketClient->connectToServer();

    // Je crée le serveur
    std::unique_ptr<SocketUnix> microWMTestServer = std::make_unique<SocketUnix>("/tmp/microWMTest.sock",true);
    microWMTestServer->createServer();

    bool haveMessage=false;
      
    std::string input;

    while(true){
        std::cout << "Enter a message to send to MicroWMserver or 'q' to quit: " << std::endl;
        getline(std::cin, input);

        if(input == "q"){
            break;
        }
        SocketMessage fMessage=parseInputToSocketMessage(input);
        microWMSocketClient->send(fMessage);
        std::cout << "argument:" <<fMessage.jsonArguments;
        // J'attend La réponse
        if(!haveMessage)
        {
            microWMTestServer->waitForClientConnection();
            haveMessage=true;
        }
        std::vector<SocketMessage> messages=microWMTestServer->receive();
        for(SocketMessage message:messages)
        {
            std::cout << message.function << "\n" << message.jsonArguments << std::endl;
            std::cout.flush();
        }

    }

    return 0;
}