#include<iostream>
#include<dirent.h>
#include<bits/stdc++.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

bool isDir(string);
string getFileName(string);
string calFileSize(long int);
long int calDirectorySize(string);
struct tm* GetTimeAndDate(unsigned long long);
string splitIntoDate(struct tm*);
void calTerminalSize();
void clearConsole();
void printScreen();
bool truncatePWD();
void dirIsNull();
void lsPWD(string);
vector<string> parseCommand(string);
string removeRedundancy(string);
string calAbsolutePath(string);
void printingCommandMode();
void create_file(vector<string>);
void create_dir(vector<string>);
void rename(vector<string>);
void copy_file(string, string);
void copy_dir(string, string);
void copy(vector<string>);
void deleteSingleFile(string);
void delete_file(vector<string>);
void deleteSingleDir(string);
void delete_dir(vector<string> parse);
void moveSingleFile(string, string);
void moveSingleDir(string, string);
void move(vector<string>);
void gotoDir(string);
bool searchFD(string, string);
void executingCommand(string);
bool commandMode();
void keyPress();
void resizing_terminal(int signum);
void exiting();
void init();

//Global variables
vector<vector<string>> contains;
int totalContainerSize;
string pwd;
int startPoint, endPoint;
int pointerIndex = 0;
int currTerminalHeight, currTerminalWidth, maxHeightInUse;
stack<string> forwardPath, backwardPath;
deque<char> currentCommand;
int modeBit = 0;
string errorMessage = "";
string currUser = "";
struct termios old_tio;

//utility function
//checking whether it is a directory or not
bool isDir(string path){
    struct stat st;
    stat(path.c_str(), &st);
    string perm = "";
    perm = perm + ( (S_ISDIR(st.st_mode)) ? "d" : "-");
    if(perm[0] == 'd')  return true;
    else    return false;
}

//retreiving filename
string getFileName(string path){
    string res = "";
    int n = path.size();
    for(int i = n-1; i >= 0; i--){
        if(path[i] == '/')  break;
        res = path[i] + res;
    }
    return res;
}

//converting units of filesize
string calFileSize(long int fileSize){
    vector<string> units = {"B", "KB", "MB", "GB"};
    int ind = 0;
    
    while(ind < 4){
        if(fileSize < 1024){
            string res = to_string(fileSize) + units[ind];
            return res;
        }
        fileSize /= 1024;   ind++;
    }
    string res = to_string(fileSize) + units[ind-1];
    return res; 

}

//recursively calculating directory size
long int calDirectorySize(string DirPath){
    long int dirSize = 0;
    const char* writable = DirPath.c_str();
    DIR *dir = opendir(writable);
    if(dir == NULL)    return 0;

    struct dirent* entity;
    entity = readdir(dir);

    while(entity != NULL){

        //filename : contains[0]
        string fileName = entity->d_name;
        string completePath = DirPath + "/" + fileName;

        //checking for file or dir
        struct stat st;
        stat(completePath.c_str(), &st);
        string perm = "";
        perm = perm + ( (S_ISDIR(st.st_mode)) ? "d" : "-");
        if(fileName[0] != '.' && perm[0] == 'd'){
            dirSize += calDirectorySize(completePath);
        }
        else if(perm[0] == '-'){
            long int fileSize = st.st_size;
            dirSize += fileSize;
        }

        //reading next file/dir
        entity = readdir(dir);
    }
    closedir(dir);
    return dirSize;
}

//using ctime
struct tm* GetTimeAndDate(unsigned long long milliseconds) {
    time_t seconds = (time_t)(milliseconds);
    return localtime(&seconds);
}

//splitting date and time
string splitIntoDate(struct tm *p){
    string day = to_string(p->tm_mday);     if(day.size() == 1)     day = "0" + day;
    string month = to_string(p->tm_mon);     if(month.size() == 1)     month = "0" + month;
    string year = to_string(p->tm_year + 1900);
    string hour = to_string(p->tm_hour);     if(hour.size() == 1)     hour = "0" + hour;
    string minutes = to_string(p->tm_min);     if(minutes.size() == 1)     minutes = "0" + minutes;
    string resDate = day + "/" + month + "/" + year + " " + hour + ":" + minutes;
    return resDate;
}

//calculating terminal window size
void calTerminalSize(){
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
    currTerminalHeight = window.ws_row;
    currTerminalWidth = window.ws_col;
    maxHeightInUse = currTerminalHeight * 0.9;
}

//clearing the screen
void clearConsole(){
    cout << "\033[H\033[2J\033[3J" ;
}

//printing on screen
void printScreen(){
    
    int i;
    int noOfElements = 0;
    for(i = startPoint; i <= endPoint; i++) {
        noOfElements++;
        if(i == pointerIndex)   {
            cout << "\033[35m";
            cout << ">>>" << "\t";
        }
        else    cout << "\t";

        string fullDetails = "";
        string name = contains[i][0];
        // printf("%-30s", name.c_str());
        if(name.size() > 26){
            fullDetails = name.substr(0, 26) + "... ";
        }
        else{
            fullDetails = name;
            for(int k = name.size() + 1; k <= 30; k++)      fullDetails += " ";
        }
        for(int j = 1; j < contains[i].size(); j++){

            fullDetails += contains[i][j];
            for(int k = contains[i][j].size() + 1; k <= 16; k++)    fullDetails += " ";
            // cout << contains[i][j] << "\t\t";
        }
        currTerminalWidth = currTerminalWidth - 9;
        if(currTerminalWidth < fullDetails.size())    fullDetails = fullDetails.substr(0, currTerminalWidth);
        cout << fullDetails;
        currTerminalWidth = currTerminalWidth + 9;
        if(i == pointerIndex)   {
            cout << "\033[0m";
        }
        cout << "\n";
    }

    for(i = noOfElements; i < currTerminalHeight; i++){
        if(i == currTerminalHeight - 1)     cout << "Normal Mode:::" << pwd;
        else    cout << endl;
    }
}

//truncating pwd to go back to parent directory
bool truncatePWD(){
    int i;
    for(i = pwd.size() - 1; i >= 0; i--){
        if(pwd[i] == '/')   break;
    }
    if(i == 0)   {
        if(pwd == "/")  return false;
        pwd = "/";
        return true;
    }
    string res = "";
    for(int j = 0; j < i; j++)      res += pwd[j];
    pwd = res;
    return true;
}

//dir is null
void dirIsNull(){
    for(int i = 5; i > 0; i--){
        clearConsole();
        cout << "Current Directory doesn't exists/doesn't have rw permissions" << "\n";
        cout << "Redirecting to parent directory in ";
        cout << "\033[31m" << "\033[4m" << i << "\033[0m" << " secs";
        for(int i = 2; i <= currTerminalHeight; i++){
        if(i == currTerminalHeight){
                if(modeBit == 0)    cout << "Normal Mode:::";
                else    cout << "Command Mode:::";
            }
            else    cout << endl;
        }
        cout.flush();
        sleep(1);
    }
    errorMessage = "~";
    truncatePWD();
    clearConsole();
    lsPWD(pwd);
}

//preprocessing directory details
void lsPWD(string path){
    //takes input in absolute path
    contains.clear();
    startPoint = 0;     endPoint = -1;
    totalContainerSize = 0;
    pointerIndex = 0;
    int index = 0;
    const char* writable = path.c_str();
    DIR *dir = opendir(writable);
    if(dir == NULL)    {
        backwardPath.pop();
        dirIsNull();
        return;
    }
    struct dirent* entity;
    entity = readdir(dir);

    while(entity != NULL){
        

        //filename : contains[0]
        string fileName = entity->d_name;
        if(fileName != "." && fileName != ".." && fileName[0] == '.'){
            //reading next file/dir
            entity = readdir(dir);
            continue;
        }
        string completePath = path + "/" + fileName;
        contains.push_back({fileName});

        
        struct stat st;
        stat(completePath.c_str(), &st);

        //file permissions : contains[4]
        string perm = "";
        perm = perm + ( (S_ISDIR(st.st_mode)) ? "d" : "-");
        perm = perm + ( (st.st_mode & S_IRUSR) ? "r" : "-");
        perm = perm + ( (st.st_mode & S_IWUSR) ? "w" : "-");
        perm = perm + ( (st.st_mode & S_IXUSR) ? "x" : "-");
        perm = perm + ( (st.st_mode & S_IRGRP) ? "r" : "-");
        perm = perm + ( (st.st_mode & S_IWGRP) ? "w" : "-");
        perm = perm + ( (st.st_mode & S_IXGRP) ? "x" : "-");
        perm = perm + ( (st.st_mode & S_IROTH) ? "r" : "-");
        perm = perm + ( (st.st_mode & S_IWOTH) ? "w" : "-");
        perm = perm + ( (st.st_mode & S_IXOTH) ? "x" : "-");


        // filesize : contains[1]
        //if(perm[0] == '-' || fileName == "." || fileName == ".." || path == "/"){
            long int fileSize = st.st_size;
            contains[index].push_back(calFileSize(fileSize));
        //}
        //else if(perm[0] == 'd'){
            //long int fileSize = calDirectorySize(completePath);
            //contains[index].push_back(calFileSize(fileSize));
        //}

        //reading user/group name : contains[2]/[3]
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        string user = pw->pw_name;
        contains[index].push_back(user);
        string group = gr->gr_name;
        contains[index].push_back(group);

        //pushing file permissions
        contains[index].push_back(perm);

        unsigned long long int modTime = st.st_mtime;
        struct tm *p = GetTimeAndDate(modTime);
        string date = splitIntoDate(p);
        contains[index].push_back(date);

        //container size increasing by 1
        totalContainerSize++;

        //reading next file/dir
        entity = readdir(dir);
        index++;
        
    }
    closedir(dir);
    sort(contains.begin(), contains.end());
    startPoint = 0;  
    totalContainerSize = contains.size();
    endPoint = totalContainerSize - 1 < maxHeightInUse - 1 ? totalContainerSize - 1 : maxHeightInUse - 1;
}

//command parsing
vector<string> parseCommand(string command){
    vector<string> parse;
    int n = command.size();
    string temp = "";
    for(int i = 0; i < n; i++){
        if(command[i] != ' ') {
            temp += command[i];
        }
        else{
            if(temp.size() != 0)    parse.push_back(temp);
            temp = "";
        }
    }
    if(temp.size() != 0)    parse.push_back(temp);
    return parse;
}

//removing redundant char in pwd
string removeRedundancy(string path){
    if(path == "/")     return path;
    path = path + "/";
    string res = "";
    res += path[0];
    int n = path.size();
    for(int i = 1; i < n; i++){
        if(path[i] == '/' && path[i-1] == '/')  continue;
        res += path[i];
    }
    path = res;
    res = "";
    string token = "";
    n = path.size();
    for(int i = 1; i < n; i++){
        if(path[i] == '/'){
            if(token == "."){
                //skip this part
            }
            else if(token == ".."){
                int ind;
                for(ind = res.size() - 1; ind >= 0; ind--){
                    if(res[ind] == '/')   break;
                }
                string temp = "";
                for(int i = 0; i < ind; i++)    temp += res[i];
                res = temp;
            }
            else{
                res = res + "/" + token;
            }
            token = "";
        }
        else{
            token += path[i];
        }
    }
    // res = res + "/" + token;
    if(res.size() == 0)     res = "/";

    return res;
}

//calculating absolute path
string calAbsolutePath(string path){
    string abs = "";
    int n = path.size();
    if(path[0] == '/'){
        abs = path;
    }
    else if(path[0] != '.' && path[0] != '~'){
        abs = pwd + "/" + path;
    }
    else if(n > 1 && path[0] == '.' && path[1] == '.'){
        int m = pwd.size(), i;
        for(i = m-1; i >= 0; i--){
            if(pwd[i] == '/')   break;
        }
        for(int j = 0; j < i; j++)      abs += pwd[j];

        for(int i = 2; i < n; i++)      abs += path[i];
        if(abs.size() == 0)     abs = "/"; 
    }
    else if(path[0] == '.'){
        if(path == ".")     abs = pwd;
        else  {
            if(pwd != "/")      abs = pwd;
            for(int i = 1; i < n; i++)      abs += path[i];
        }
    }
    else if(path[0] == '~'){
        abs = "/home/santanu";
        for(int i = 1; i < n; i++){
            abs += path[i];
        }
    }
    abs = removeRedundancy(abs);
    return  abs;
}

//printing command mode
void printingCommandMode(){
    int i;
    int noOfElements = 0;
    for(i = startPoint; i <= endPoint; i++) {
        noOfElements++;
        if(i == pointerIndex)   {
            cout << "\033[35m";
            cout << ">>>" << "\t";
        }
        else    cout << "\t";
        
        string fullDetails = "";

        string name = contains[i][0];
        // printf("%-30s", name.c_str());
        if(name.size() > 26){
            fullDetails = name.substr(0, 26) + "... ";
        }
        else{
            fullDetails = name;
            for(int k = name.size() + 1; k <= 30; k++)      fullDetails += " ";
        }
        for(int j = 1; j < contains[i].size(); j++){

            fullDetails += contains[i][j];
            for(int k = contains[i][j].size() + 1; k <= 16; k++)    fullDetails += " ";
            // cout << contains[i][j] << "\t\t";
        }
        currTerminalWidth = currTerminalWidth - 9;
        if(currTerminalWidth < fullDetails.size())    fullDetails = fullDetails.substr(0, currTerminalWidth);
        cout << fullDetails;
        currTerminalWidth = currTerminalWidth + 9;
        if(i == pointerIndex)   {
            cout << "\033[0m";
        }
        cout << "\n";
    }

    for(i = noOfElements; i < currTerminalHeight; i++){
        if(i == currTerminalHeight - 2)     {
            cout << "Command Mode:::";
            if(errorMessage == "Success!!" || errorMessage == "True"){
                cout << "\033[32m";
                cout << errorMessage;
                cout << "\033[0m";
            }
            else{
                cout << "\033[31m";
                cout << errorMessage;
                cout << "\033[0m";
            }

            cout << "\n";
        }
        else if(i == currTerminalHeight - 1)    cout << pwd << "$ ";
        else    cout << endl;
    }

    int n = currentCommand.size();
    string command = "";
    while(n--){
        command += currentCommand.front();
        char c = currentCommand.front();
        currentCommand.pop_front();
        currentCommand.push_back(c);
    }
    for(int i = 0; i < command.size(); i++)      cout << command[i];
}

//Implementing system commands
//implementing create_file command
void create_file(vector<string> parse){

    int n = parse.size();
    string destDir;
    int end;
    if(n == 2){
        destDir = pwd;
        errorMessage = "New files created in " + destDir;
        end = 1;
    }
    else{
        destDir = calAbsolutePath(parse[n-1]);
        end = n-2;
    }
    //traversing each file
    for(int i = 1; i <= end; i++){
        string completePath = destDir + "/" + parse[i];
        int flag = open(completePath.c_str(), O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(flag == -1){
            errorMessage = "Error in creating file in " + completePath + "!!";
        }
        // close(completePath.c_str());
    }
}

//create directory
void create_dir(vector<string> parse){

    int n = parse.size();
    string destDir;
    int end;
    if(n == 2){
        destDir = pwd;
        errorMessage = "New folder created in " + destDir;
        end = 1;
    }
    else{
        destDir = calAbsolutePath(parse[n-1]);
        end = n-2;
    }
    //traversing each folder to be created
    for(int i = 1; i <= end; i++){
        string completePath = destDir + "/" + parse[i];
        int flag = mkdir(completePath.c_str(), 0777);
        if(flag == -1){
            errorMessage = "Error in creating folder in " + completePath + "!!";
        }
    }
}

//renaming file
void rename(vector<string> parse){
    string initial = parse[1], modified = parse[2];
    string name1 = "", name2 = "";
    int ind1;
    for(ind1 = initial.size() - 1; ind1 >= 0; ind1--){
        if(initial[ind1] == '/')   break;
        name1 = initial[ind1] + name1;
    }
    string resDir = "";
    for(int i = 0; i < ind1; i++){
        resDir += initial[i];
    }
    if(ind1 == 0)  resDir = "/";
    else if(resDir.size() == 0)  resDir = ".";
    resDir = calAbsolutePath(resDir);

    for(int i = modified.size() - 1; i >= 0; i--){
        if(modified[i] == '/')   break;
        name2 = modified[i] + name2;
    }
    initial = resDir + "/" + name1;
    modified = resDir + "/" + name2;

    int flag = rename(initial.c_str(), modified.c_str());
    if(flag != 0){
        errorMessage = "Error while renaming!!";
    }
}

//copying single file
void copy_file(string source, string dest){
    
    dest = dest + "/" + getFileName(source);      //appending file name at the end
    int fileDesSrc, fileDesDest;
    fileDesSrc = open(source.c_str(), O_RDONLY);
    fileDesDest = open(dest.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    char buff[1024];   //byte by byte data read
    int noOfBytesRead;
    while ((noOfBytesRead = read(fileDesSrc, buff, sizeof(buff))) > 0){
        write(fileDesDest, buff, noOfBytesRead);
    }

    struct stat st;
    stat(source.c_str(), &st);
    int flag;

    if((flag = chmod(dest.c_str(), st.st_mode)) != 0){
        errorMessage = "Error while setting up file permissions/ownership!!";
        return;
    }
    if((flag = chown(dest.c_str(), st.st_uid, st.st_gid)) != 0){
        errorMessage = "Error while setting up file permissions/ownership!!";
        return;
    }
    
}

//copying directory
void copy_dir(string source, string dest){

    dest = dest + "/" + getFileName(source);
    int flag = mkdir(dest.c_str(), 0777);
    if(flag == -1){
        errorMessage = "Error in creating folder in " + dest + "!!";
        return;
    }
    struct stat st;
    stat(source.c_str(), &st);
    if((flag = chmod(dest.c_str(), st.st_mode)) != 0){
        errorMessage = "Error while setting up file permissions/ownership!!";
        return;
    }
    if((flag = chown(dest.c_str(), st.st_uid, st.st_gid)) != 0){
        errorMessage = "Error while setting up file permissions/ownership!!";
        return;
    }

    const char* writable = source.c_str();
    DIR *dir = opendir(writable);
    if(dir == NULL)    return;

    struct dirent* entity;
    entity = readdir(dir);

    while(entity != NULL){

        //filename : contains[0]
        string name = entity->d_name;
        string completePath = source + "/" + name;

        if(name == "." || name == ".."){
            //do nothing
        }
        else if(isDir(completePath)){
            //copy dir recursively
            copy_dir(completePath, dest);
        }
        else{
            //copy a single file
            copy_file(completePath, dest);
        }

        //reading next file/dir
        entity = readdir(dir);
    }
    closedir(dir);
}

//copy command
void copy(vector<string> parse){

    int n = parse.size();
    string dest = calAbsolutePath(parse[n-1]);
    for(int i = 1; i <= n-2; i++){
        string source = calAbsolutePath(parse[i]);

        if(isDir(source)){
            copy_dir(source, dest);
        }
        else{
            copy_file(source, dest);
        }
    }
}

//delete file
void deleteSingleFile(string source){
    int flag = remove(source.c_str());
    if(flag != 0){
        errorMessage = "Unable to delete file!!";
    }
}

//delete_file command
void delete_file(vector<string> parse){
    int n = parse.size();
    for(int i = 1; i < n; i++){
        string source = calAbsolutePath(parse[i]);
        deleteSingleFile(source);
    }
}

//delete dir
void deleteSingleDir(string source){

    if(source == "/"){
        errorMessage = "Failure!! Don't try this at home!!";
        return;
    }
    const char* writable = source.c_str();
    DIR *dir = opendir(writable);
    if(dir == NULL)    return;

    struct dirent* entity;
    entity = readdir(dir);

    while(entity != NULL){

        //filename : contains[0]
        string name = entity->d_name;
        string completePath = source + "/" + name;

        if(name == "." || name == ".."){
            //do nothing
        }
        else if(isDir(completePath)){
            //copy dir recursively
            deleteSingleDir(completePath);
        }
        else{
            //copy a single file
            deleteSingleFile(completePath);
        }

        //reading next file/dir
        entity = readdir(dir);
    }
    closedir(dir);

    int flag = rmdir(source.c_str());
    if(flag != 0){
        errorMessage = "Failed to remove directory!!";
        return;
    }
}

//delete_dir command
void delete_dir(vector<string> parse){
    int n = parse.size();
    for(int i = 1; i < n; i++){
        string source = calAbsolutePath(parse[i]);
        deleteSingleDir(source);
    }
}

//move single file
void moveSingleFile(string source, string dest){
    copy_file(source, dest);
    if(errorMessage.size() != 0){
        errorMessage = "Unable to move file!!";
        return;
    }
    deleteSingleFile(source);
}

//move single directory
void moveSingleDir(string source, string dest){
    copy_dir(source, dest);
    if(errorMessage.size() != 0){
        errorMessage = "Unable to move directory!!";
        return;
    }
    deleteSingleDir(source);
}

//move command
void move(vector<string> parse){
    int n = parse.size();
    string dest = calAbsolutePath(parse[n-1]);
    for(int i = 1; i <= n-2; i++){
        string source = calAbsolutePath(parse[i]);

        if(isDir(source)){
            moveSingleDir(source, dest);
        }
        else{
            moveSingleFile(source, dest);
        }
    }
}

//goto Directory
void gotoDir(string path){
    backwardPath.push(pwd);
    pwd = calAbsolutePath(path);
}

//search file/dir
bool searchFD(string searchPwd, string name){

    const char* writable = searchPwd.c_str();
    DIR *dir = opendir(writable);
    if(dir == NULL)    return false;

    struct dirent* entity;
    entity = readdir(dir);

    while(entity != NULL){

        //filename : contains[0]
        string fileName = entity->d_name;
        if(fileName == name)    return true;
        string completePath = searchPwd + "/" + fileName;

        if(fileName == "." || fileName == ".." || fileName[0] == '.'){
            //do nothing
        }
        else if(isDir(completePath)){
            if(searchFD(completePath, name))    return true;
        }

        //reading next file/dir
        entity = readdir(dir);
    }
    closedir(dir);
    return false;
}

//executing command
void executingCommand(string command){
    vector<string> parse;
    parse = parseCommand(command);
    errorMessage = "";
    int n = parse.size();
    if(n == 0){
        errorMessage = "Invalid command!!";
    }
    else if(parse[0] == "copy"){
        if(n < 3){
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            copy(parse);
            lsPWD(pwd);
        }
    }
    else if(parse[0] == "move"){
        if(n < 3){
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            move(parse);
            lsPWD(pwd);
        }
    }
    else if(parse[0] == "rename"){
        if(n != 3){
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            rename(parse);
            lsPWD(pwd);
        }
    }
    else if(parse[0] == "create_file"){
        if(n < 2){
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            create_file(parse);
            lsPWD(pwd);
        }
    }
    else if(parse[0] == "create_dir"){
        if(n < 2){
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            create_dir(parse);
            lsPWD(pwd);
        }
    }
    else if(parse[0] == "delete_file"){
        if(n < 2){
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            delete_file(parse);
            lsPWD(pwd);
        }
    }
    else if(parse[0] == "delete_dir"){
        if(n < 2){
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            delete_dir(parse);
            lsPWD(pwd);
        }
    }
    else if(parse[0] == "goto"){
        if(n != 2)   {
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            gotoDir(parse[1]);
            lsPWD(pwd);
        }
    }
    else if(parse[0] == "search"){
        if(n != 2){
            errorMessage = "Invalid number of arguments!!";
        }
        else{
            bool flag = searchFD(pwd, parse[1]);
            errorMessage = flag == 1 ? "True" : "False";
        }     
    }
    else{
        errorMessage = "Invalid command!!";
    }

    if(errorMessage.size() == 0)    errorMessage = "Success!!";
}

//entering command mode
bool commandMode(){
    clearConsole();
    while(!currentCommand.empty())      currentCommand.pop_back();
    printingCommandMode();

    char input;
    while(input=getchar()){
        
        if(input == 27){            //esc keypress
            errorMessage = "";
            return false;
        }
        else if(input == 13){     //enter keypress
            errorMessage = "Executing command...";
            clearConsole();
            printingCommandMode();
            string command = "";
            while(!currentCommand.empty()){
                command += currentCommand.front();
                currentCommand.pop_front();
            }
            //evaluating entered command
            if(command == "quit")   return true;
            
            executingCommand(command);
            clearConsole();
            printingCommandMode();
        }
        else if(input == 127){      //backspace keypress
            clearConsole();
            if(!currentCommand.empty())     currentCommand.pop_back();
            printingCommandMode();
        }
        else{
            currentCommand.push_back(input);
        }
    }
    return false;
}

//keypress event in normal mode
void keyPress(){
    char input;
    while(input=getchar()){

        clearConsole();
        printScreen();
        if(input == 'D'){           //left keypress

            if(backwardPath.empty())    {
                clearConsole();
                printScreen();
                continue;
            }

            clearConsole();
            forwardPath.push(pwd);
            pwd = backwardPath.top();
            backwardPath.pop();
            pointerIndex = 0;
            lsPWD(pwd);
            printScreen();
        }
        else if(input == 'A'){      //up keypress
            clearConsole();
            pointerIndex = pointerIndex - 1 < 0 ? 0 : pointerIndex - 1;
            if(pointerIndex == startPoint - 1 && startPoint != 0){
                startPoint--;
                endPoint--;
            }
            printScreen();
        }
        else if(input == 'C'){      //right keypress

            if(forwardPath.empty())     {
                clearConsole();
                printScreen();
                continue;
            }
            
            clearConsole();
            backwardPath.push(pwd);
            pwd = forwardPath.top();
            forwardPath.pop();
            pointerIndex = 0;
            lsPWD(pwd);
            printScreen();
        }
        else if(input == 'B'){      //down keypress
            clearConsole();
            pointerIndex = pointerIndex + 1 >= totalContainerSize ? totalContainerSize - 1 : pointerIndex + 1;
            if(pointerIndex == endPoint + 1 && endPoint != totalContainerSize - 1){
                startPoint++;
                endPoint++;
            }
            printScreen();
        }
        else if(input == 13){     //enter keypress
            
            char isDir = contains[pointerIndex][4][0];
            string fileName = contains[pointerIndex][0];
            string resPath = pwd + "/" + fileName;
            if(pwd == "/")  resPath = pwd + fileName;
            
            if(isDir == 'd'){         //opening a directory
                if(fileName == "."){
                    clearConsole();
                    printScreen();
                    continue;
                }
                else if(fileName == ".."){
                    
                    string temp = pwd;
                    clearConsole();
                    if(truncatePWD())   {
                        backwardPath.push(temp); 
                        pointerIndex = 0;
                    }
                    lsPWD(pwd);
                    printScreen();
                }
                else{
                    backwardPath.push(pwd); 
                    pwd = resPath;
                    pointerIndex = 0;
                    clearConsole();
                    lsPWD(pwd);
                    printScreen();
                }
                
            }
            else{                   //opening a file
                
                pid_t childId = fork();
                if (childId == -1) {
                    cout << "Error in opening file, please try again!!";
                    exit(EXIT_FAILURE);
                }           
                else if (childId > 0) {
                    clearConsole();
                    printScreen();
                    continue;
                } 
                else {
                    
                    execl("/usr/bin/xdg-open","xdg-open",resPath.c_str(),NULL);
                    exit(EXIT_SUCCESS);
                }
            }
        }
        else if(input == 127){      //backspace keypress

            backwardPath.push(pwd);

            clearConsole();
            if(truncatePWD())   pointerIndex = 0;
            else    backwardPath.pop();
            lsPWD(pwd);
            printScreen();
        }
        else if(input == 104){      //home keypress

            backwardPath.push(pwd);

            clearConsole();
            pwd = "/home/" + currUser;
            pointerIndex = 0;
            lsPWD(pwd);
            printScreen();
        }
        else if(input == 'q'){      //quit keypress
            clearConsole();
            return;
        }
        else if(input == 58){       //colon keypress, entering command mode
            modeBit = 1;
            if(commandMode()){
                clearConsole();
                return;
            }
            modeBit = 0;
            clearConsole();
            lsPWD(pwd);
            printScreen();
        }
        else{
            clearConsole();
            printScreen();
        }
    }
}

//horizontal resizing
void resizing_terminal(int signum){
    calTerminalSize();
    lsPWD(pwd);
    pointerIndex = startPoint;
    clearConsole();
    if(modeBit == 0)    printScreen();
    else    printingCommandMode();
    cout.flush();
}

//on exit
void exiting(){
    tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);
}

//initialization
void init(){
    //entering raw mode
    struct termios new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_iflag &= (~ICRNL);
    new_tio.c_lflag &=~(ICANON | ISIG);
    tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);
    atexit(exiting);
    signal(SIGWINCH, resizing_terminal);
    
    //fetching curr user
    uid_t id = geteuid();
    struct passwd *pw = getpwuid(id);
    currUser = pw->pw_name;
    pwd = "/home/" + currUser;

    clearConsole();
    calTerminalSize();
    lsPWD(pwd);
    printScreen();
    keyPress();
}

int main(){
    init();
    return 0;
}
