
#include <boost/filesystem.hpp>

#include"ModelDesignerServer.h"

int main() {

    ModelDesignerServer server;

    if (!server.start(8080, 8081)) {

        std::cerr << "Failed to start server" << std::endl;

        return 1;

    }

    std::cout << "Server is running. Press Enter to stop..." << std::endl;

    std::cin.get();

    server.stop();

    return 0;

}
