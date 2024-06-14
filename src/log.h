
#include <string>
#include <fstream>
#include <ostream>

std::string fp_log = "/Users/daviddoellstedt/Documents/GitHub/venus_chess/log1.txt";

void printAndWriteToLog(std::string str) {
  std::ofstream outfile;
  outfile.open(fp_log, std::ios_base::app);
  outfile << "<< " << str << "\n";
  outfile.close();
  
  std::cout << str << std::endl;
}
