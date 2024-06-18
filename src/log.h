
#include <string>
#include <fstream>
#include <ostream>

std::string fp_log = "/Users/daviddoellstedt/Documents/GitHub/venus_chess/log1.txt";
bool logging = false;

/** Prints input to std out and to a log file. Useful for debugging UCI GUIs.
 *
 * @param str: Test to print and log.
 */
void printAndWriteToLog(std::string str) {
  if (logging) {
    std::ofstream outfile;
    outfile.open(fp_log, std::ios_base::app);
    outfile << "<< " << str << "\n";
    outfile.close();
  }

  std::cout << str << std::endl;
}
