#include <iostream>
#include <poll.h>
#include <memory>
#include "modules/XCBEventsManager.h"
#include "modules/SocketUnix.h"
#include "modules/logManager.h"
#include <thread>

int main()
{
	// J'initialise la gestion des logs
	LogManager logManager(true);
	logManager.addLog(Log("Bienvenue sur MicroWm",LogSeverity::Info,"microWm->main"));
	// J'initialise le gestionnaire d'évenements XCB
	std::unique_ptr<XCBEventsManager> xcbEventsManager=std::make_unique<XCBEventsManager>(logManager);
	// J'initialise le serveur de socket Unix
	std::unique_ptr<SocketUnix> microWMSocketServer = std::make_unique<SocketUnix>("/tmp/microWM.sock",true);
    microWMSocketServer->createServer();

	// Je me connecte au serveur de test
	std::unique_ptr<SocketUnix> microWMTestClient = std::make_unique<SocketUnix>("/tmp/microWMTest.sock",false);
	bool haveMessage=false;

	// Je crée un pollfd pour surveiller les évènements X et Dbus
	struct pollfd fds[2];
	fds[0].fd = xcbEventsManager->getFileDescriptor();
	fds[0].events = POLLIN;
	fds[1].fd = microWMSocketServer->getFileDescriptor();
	fds[1].events = POLLIN;

	while (poll(fds, 2, -1) > 0)
	{
		// Si la connexion socket client est fermée
		if (fds[1].revents & POLLHUP)
		{
			std::cout << "connexion fermée\n";
			// Je réinitialise la connexion
			microWMSocketServer->switchConnection();
			// Je réinitialise le client
			microWMTestClient = std::make_unique<SocketUnix>("/tmp/microWMTest.sock",false);
			// Je met le descripteur de fichier à jour
			fds[1].fd = microWMSocketServer->getFileDescriptor();
			fds[1].events = POLLIN;
			haveMessage=false;
		}
		else if (fds[0].revents & POLLIN)
		{
			// Boucle d'evenements XCB
			xcbEventsManager->handleEvent();
		}
		else if (fds[1].revents & POLLIN)
		{
			if(!haveMessage)
			{
				microWMSocketServer->waitForClientConnection();
				fds[1].fd=microWMSocketServer->getFileDescriptor();
				microWMTestClient->connectToServer();
				haveMessage=true;
			}

			std::vector<SocketMessage> messages= microWMSocketServer->receive();
			for (SocketMessage message:messages)
			{
				if(message.function=="exit")
				{
					exit(1);
				}
				if(message.function=="getWindows")
				{
					microWMTestClient->send("windows",xcbEventsManager->getXcbManager()->getWindowsForSocket());
				}
				else if(message.function=="getAllLogs")
				{
					microWMTestClient->send("logs",logManager.getAllLogsForSocket());
				}
				else if(message.function=="getDisplaysConfigurations")
				{
					microWMTestClient->send("display",xcbEventsManager->getXcbManager()->getDisplayConfigurationForSocket());
				}
				else if(message.function=="applyGeometryConfiguration")
				{
					xcbEventsManager->getXcbManager()->applyGeometryConfiguration(message.jsonArguments);
					microWMTestClient->send("applyGeometryConfiguration","Done");
				}
				else if(message.function=="setToDesktop")
				{
					xcbEventsManager->getXcbManager()->setToDesktop(stoi(message.jsonArguments));
					microWMTestClient->send("setToDesktop","Done");
					logManager.addLog(Log("Conversion de la fenêtre "+message.jsonArguments+" en Desktop",LogSeverity::Info,"microWM-main"));
				}
				else if(message.function=="setToDock")
				{
					xcbEventsManager->getXcbManager()->setToDock(stoi(message.jsonArguments));
					microWMTestClient->send("setToDock","Done");
					logManager.addLog(Log("Conversion de la fenêtre "+message.jsonArguments +" en Dock",LogSeverity::Info,"microWM-main"));
				}
				else if(message.function=="setDockHeight")
				{
					xcbEventsManager->getXcbManager()->setDockHeight(stoi(message.jsonArguments));
					microWMTestClient->send("SetDockHeight"," "+message.jsonArguments);
					logManager.addLog(Log("Changement de la hauteur du dock:"+message.jsonArguments +"px",LogSeverity::Info,"microWM-main"));
				}
				else
				{
					microWMTestClient->send("Attention! ","Fonction non gérée");
				}
			}
		}
	}
	return 0;
}