#include <iostream>
#include <stdio.h>
#include <bits/stdc++.h>
#include <string.h>
#include <fstream>
#include <streambuf>

#define BF_INTERPRETER_VERSION 1
#define DEFAULT_BF_MEMORY 30000
#define DEFAULT_DUMP_MEMORY 32

using namespace std;

unsigned char* bf_memory; // Stored as a pointer to dinamically initialize it
unsigned char* code_memory;
int bf_memory_size;
int code_memory_size;
int memory_cursor; // Current in-memory position

int debug_mode = 0;
int verbose_mode = 0;
int size_memory_dump = DEFAULT_DUMP_MEMORY;
int size_brainfuck_memory = DEFAULT_BF_MEMORY;

int parseArgs(int argc, char** argv);
string cleanCode(string code);
int initializeBrainfuckInterpreter(int memorySize);
int interprete(int code_cursor);
int setValueInBFMemory(int cursor, int value);
char getValueInBFMemory(int cursor);
int dumpMemory(int size);
int showHelp();
int setMemoryCursor(int newValue);
int getMemoryCursor();
char getValueInCodeMemory(int cursor);
int setValueInCodeMemory(int cursor, int value); // This function won't be used but... it is ok
int getBFMemorySize();
int getCodeMemorySize();

int showHelp(){
    cout << "--- Brainfuck Interpreter (bit) v." << BF_INTERPRETER_VERSION << " ---\n";
    cout << "This tool has two modes: by arguments and interactive.\n";
    cout << "If you don't enter arguments or if you don't enter Brainfuck code in\n";
    cout << "the arguments (with -t or -f), the interactive mode will run.\n\n";
    cout << "Arguments:\n";
    cout << "-h: It shows this help.\n";
    cout << "-v: Verbose mode, it shows all the program outputs (except debug output if it is not enabled).\n";
    cout << "-t CODE: It runs the Brainfuck code. You have to enter the CODE without spaces nor newlines in this mode.\n";
    cout << "-f PATH_TO_FILE: It runs the Brainfuck code inside a file.\n";
    cout << "-m INT: It sets the size of the Brainfuck interpreters memory. Not recommended less than 30000. (It represents 30000 bytes)\n";
    cout << "-s INT: It sets the size of the memory dumps (in debug mode). By default 32.\n";
    cout << "-i: (Not implemented yet!) It uses dynamic memory so no need to specify the Brainfuck interpreters memory (it overrides -m arg).\n";
    cout << "-o: (Not implemented yet!) It optimizes the code before running it. For big Brainfuck programs this option is recommended.\n\n";
    cout << "For any other question or issue check the GitHub repo: https://github.com/NauCode/BrainfuckTools\n";
    return 0;
}

// Function: main
// Args: 2, argc, an int with the number of args; and argv, a char** with the args
// What does it do? It is the main function to run the interpreter
// Return: An int with the result code of the program
// Valid args:
// Just run the program with -h to see all the valid args
// Return codes:
// 0: Program run fine
// -1: Invalid args
// -2: Problem reading file specified in args
// -999: Unknown error happened
int main(int argc, char** argv) {

    string code = "";

    int parseResult = parseArgs(argc,argv);

    switch(parseResult){
        case 0:
            // Nothing to do, code is loaded, everything ready to run
            // This is here just for... beauty?
            break;
        case -1:
            // We got args but no code!!
            // So we will ask the user for it
        case -2:
            // In this case the users called the program without args
            // So we will ask the user for the code to run
            if(verbose_mode) {
                cout << "Set the code you want to run:\n";
            }

            // Reads input with spaces!
            getline(cin >> ws, code);

            code = cleanCode(code);
            code_memory_size = code.length();
            code_memory = static_cast<unsigned char *>(malloc(code_memory_size * sizeof(char)));

            // We have to set all the Brainfuck interpreter memory to zeroes
            // I don't like doing it with this loop, maybe later I will research a better way
            for(int i=0; i<code_memory_size; i++){
                code_memory[i] = code.at(i);
            }
            break;
        case -3:
            // In this case maybe we sent some args but we also printed the help menu, so
            // we end the program after showing it, without running anything more
            return 0; // Since we are in the main function, I can end the program like this
            // exit(0); But I could use this!
            break; // And yeah, this break is just because I want to close all the case instructions
            // with break;, but it will never run!
        case -4:
            // In this case we entered some invalid args causing the program to don't run
            // We could just fix/skip those args but... I prefer this way
            return -1; // We end the program with error -1
            // exit(-1); This would also work
            break;
        case -5:
            // In this case we were not able to read the file user send as arg in -f PATH_TO_FILE
            return -2; // We end the program with error -2
            // exit(-2);
            break;
        default:
            // The default case should never happen, but since it should never happen, if it happens
            // it has to be an error, so we handle it like an error!
            return -999; // An unknown error
            // exit(-999); ?
            break;
    }

    // Once args are done, we have to initialize the interpreter
    if(initializeBrainfuckInterpreter(size_brainfuck_memory)!=0){
        if(debug_mode) {
            cout << "[DEBUG] " << "Error: We had some error initializing Brainfuck interpreter!\n";
        }
        // If we had some error initializing it, we can't run the Brainfuck programs
        // So we exit with error -2
        return -2;
        // exit(-2); As you prefer
    }else{
        // I am not sure if this should go as verbose or debug
        if(verbose_mode) {
            cout << "Brainfuck interpreter initialized!\n";
        }
    }

    // If we are debugging a program, we want to print the initial interpreter memory
    // Full of zeros of course
    if(debug_mode){
        dumpMemory(size_memory_dump);
    }

    // I usually like to declare all vars at the beginning of the function
    // (Except temporal vars)
    // But I think this goes better here
    int code_cursor = 0;
    while(code_cursor>=0 && code_cursor<getCodeMemorySize()){
        code_cursor = interprete(code_cursor);
    }

    // If everything went fine we can end the program with no errors
    return 0;
    // exit(0); ?
}

// Function: parseArgs
// Args: 2, argc, an int with the number of args; and argv, a char** with the args
// What does it do? It parses the args we got when the program was launched and runs
// the program based on them
// Return: An int with the result code of the process
// Return codes:
// 0: Everything went fine!
// -1: We got args but no code! (no -f nor -t or invalid ones)
// -2: Program called without args
// -3: Help menu was printed!
// -4: Some invalid args were entered
// -5: We had troubles reading the file the user sent in -f command
int parseArgs(int argc, char** argv){
    // We have to declarate these two vars here
    // since they can't be declared in case
    ifstream fFile;
    string code = "";
    //
    int next_mustnt_arg = 0; // It 1, the next argv[argi] has to be something
    // not starting by -
    char next_mustnt_arg_from; // The command the next not command arg is from

    for(int argi = 1; argi<argc;argi++){

        // Since switch doesn't work with 'strings', we have to remove the '-' from the args
        // So the args are just chars

        // To do that, first we have to be sure it is a command (something starting with -)
        // If not, it will go to the default filter so we will worry about it later

        char parsedArg;

        // It is an arg like -d or -v
        if(strlen(argv[argi])==2 && argv[argi][0]=='-'){
            parsedArg = argv[argi][1];
        }else{
            parsedArg = 'X'; // We set this as a not valid arg, just a little trick
        }

        switch(parsedArg){
            case 'd':
                debug_mode=1;
                break;
            case 'v':
                verbose_mode=1;
                break;
            case 't':
                if(code!=""){
                    cout << "Error: You already set the code source!\n";
                    return -3;
                }
                next_mustnt_arg=1;
                next_mustnt_arg_from='t';
                break;
            case 'f':
                if(code!=""){
                    cout << "Error: You already set the code source!\n";
                    return -3;
                }
                next_mustnt_arg=1;
                next_mustnt_arg_from='f';
                break;
            case 'm':
                next_mustnt_arg=1;
                next_mustnt_arg_from='m';
                break;
            case 's':
                next_mustnt_arg=1;
                next_mustnt_arg_from='s';
                break;
            case 'h':
                // Shows the help
                showHelp();
                return -3; // This return will indicate the program to end (since you just wanted to see the help)
                break; // Not really necessary this break, I just keep it since I like all the case statements with their break
            default:
                if(next_mustnt_arg==1){
                    switch(next_mustnt_arg_from){
                        case 't':
                            code = cleanCode(argv[argi]);
                            code_memory_size = code.length();
                            code_memory = static_cast<unsigned char *>(malloc(code_memory_size * sizeof(char)));

                            for(int i=0; i<code_memory_size; i++){
                                code_memory[i] = code.at(i);
                            }
                            break;
                        case 'f':
                            // Load the file and parse the code
                            fFile.open(argv[argi]);
                            if(fFile){
                                stringstream buffer;
                                buffer << fFile.rdbuf();
                                code = cleanCode(buffer.str());
                                code_memory_size = code.length();
                                code_memory = static_cast<unsigned char *>(malloc(code_memory_size * sizeof(char)));

                                for(int i=0; i<code_memory_size; i++){
                                    code_memory[i] = code.at(i);
                                }
                            }else{
                                cout << "Error: There was some error reading the input file!\n";
                                return -5;
                            }
                            break;
                        case 'm':
                            // Not using atoi since some systems have problems with it
                            sscanf(argv[argi], "%d", &size_brainfuck_memory);
                            break;
                        case 's':
                            // Again not using atoi
                            sscanf(argv[argi], "%d", &size_memory_dump);
                            break;
                        default:
                            // We should never reach this point
                            break;
                    }
                    next_mustnt_arg = 0;
                }else{
                    cout << "Error: Invalid args!\n";
                    return -4;
                }
                break;

        }
    }

    if(code==""){
        if(argc>1){
            cout <<"No code entered! Use -t or -f next time!\n";
            return -1;
        }else{
            return -2;
        }
    }

    return 0;
}

// Function: cleanCode
// Args: 1, code, a string with the Brainfuck code we want to clean
// What does it do? It cleans a string. It removes all the characters Brainfuck is
// not able to run (so it will remove new lines, spaces and any unknown char)
// Return: A string, with the cleaned code
string cleanCode(string code){
    string new_code = "";
    for(int i=0;i<code.length();i++){
        char cI = code.at(i);
        if(cI=='>'||cI=='<'||cI=='+'||cI=='-'||cI==','||cI=='.'||cI=='['||cI==']'){
            new_code+=cI;
        }
    }
    return new_code;
}

// Function: interprete
// Args: 1, code_cursor, an int with the current char we are trying to run in the
// code_memory array.
// What does it do? It interpretes the current instruction and run it
// in the Brainfuck memory
// Return: An int with the next position of the code_cursor var
int interprete(int code_cursor){
    // This var has to be declarated here since it is not allowed in case
    int user_input;

    // This is the returning code_cursor position
    // Usually it will be just the next one
    int code_cursor_ret = code_cursor+1; // By default returns code_cursor + 1

    // This is the current instruction to run
    unsigned char order = getValueInCodeMemory(code_cursor);

    switch(order){
        case '>':
            if(debug_mode){
                cout  << "[DEBUG] " << ">: Moving from " << getMemoryCursor() << " to " << getMemoryCursor()+1 << "\n";
            }
            setMemoryCursor(getMemoryCursor()+1);
            break;
        case '<':
            if(debug_mode){
                cout << "[DEBUG] "  << "<: Moving from " << getMemoryCursor() << " to " << getMemoryCursor()-1 << "\n";
            }
            setMemoryCursor(getMemoryCursor()-1);
            break;
        case '+':
            if(debug_mode){
                cout << "[DEBUG] "  << "+: Adding 1 at " << getMemoryCursor() << " from " << (static_cast<unsigned int>(getValueInBFMemory(getMemoryCursor())) & 0xFF) << " to " << getValueInBFMemory(getMemoryCursor())+1 << "\n";
            }
            setValueInBFMemory(getMemoryCursor(), getValueInBFMemory(getMemoryCursor())+1);
            break;
        case '-':
            if(debug_mode){
                cout << "[DEBUG] "  << "-: Removing 1 at " << getMemoryCursor() << " from " << (static_cast<unsigned int>(getValueInBFMemory(getMemoryCursor())) & 0xFF) << " to " << getValueInBFMemory(getMemoryCursor())-1 << "\n";
            }
            setValueInBFMemory(memory_cursor, getValueInBFMemory(getMemoryCursor())-1);
            break;
        case '.':
            if(debug_mode){
                cout << "[DEBUG] "  << ".: Printing value at " << getMemoryCursor() << ": int()" << (static_cast<unsigned int>(getValueInBFMemory(getMemoryCursor())) & 0xFF) << " char():"<<getValueInBFMemory(getMemoryCursor())<<"\n";
            }
            cout << getValueInBFMemory(getMemoryCursor());
            break;
        case ',':
            // Important the space before %c!!!
            //scanf(" %c",&user_input);
            user_input=getchar();
            setValueInBFMemory(getMemoryCursor(), user_input);
            if(debug_mode){
                cout << "[DEBUG] "  << ",: Getting input: " << user_input << " and saving at " << getMemoryCursor() << "\n";
            }
            break;
        case '[':
            if(getValueInBFMemory(getMemoryCursor())==0){
                int tmp_cursor = code_cursor+1;
                int close_bracket_found = 0;
                int current_depth = 0;
                while(code_cursor<getCodeMemorySize() && close_bracket_found == 0){
                    unsigned char current_tmp_char = getValueInCodeMemory(tmp_cursor);
                    if(current_tmp_char=='['){
                        current_depth+=1;
                    }else if(current_tmp_char==']'){
                        if(current_depth==0){
                            // We found the closing bracket of this loop!
                            close_bracket_found=1;
                        }else{
                            current_depth-=1;
                        }
                    }

                    if(close_bracket_found!=1){
                        tmp_cursor+=1;
                    }
                }
                if(debug_mode){
                    cout << "[DEBUG] "  << "[: Since memory at " << getMemoryCursor() << " is 0, we jump to " << tmp_cursor+1 << " in code memory\n";
                }
                code_cursor_ret = tmp_cursor+1;
            }else{
                if(debug_mode){
                    cout << "[DEBUG] "  << "[: Since memory at " << getMemoryCursor() << " is not 0, we do nothing!\n";
                }
            }
            break;
        case ']':
            if(getValueInBFMemory(getMemoryCursor())!=0){
                int tmp_cursor = code_cursor-1;
                int open_bracket_found = 0;
                int current_depth = 0;
                while(tmp_cursor>=0 && open_bracket_found == 0){
                    unsigned char current_tmp_char = getValueInCodeMemory(tmp_cursor);
                    if(current_tmp_char==']'){
                        current_depth+=1;
                    }else if(current_tmp_char=='['){
                        if(current_depth==0){
                            // We found the closing bracket of this loop!
                            open_bracket_found=1;
                        }else{
                            current_depth-=1;
                        }
                    }

                    if(open_bracket_found!=1){
                        tmp_cursor-=1;
                    }
                }
                if(debug_mode){
                    cout << "[DEBUG] "  << "]: Since memory at " << getMemoryCursor() << " is not 0, we jump to " << tmp_cursor+1 << " in code memory\n";
                }
                code_cursor_ret = tmp_cursor+1;
            }else{
                if(debug_mode){
                    cout << "[DEBUG] "  << "]: Since memory at " << getMemoryCursor() << " is 0, we do nothing!\n";
                }
            }
            break;
        default:
            if(debug_mode){
                cout << "[DEBUG] "  << "Not valid instruction found, skipping...\n";
            }
            break;
    }

    if(debug_mode){
        dumpMemory(size_memory_dump);
    }

    return code_cursor_ret;
}

// Brainfuck interpreters use a memory with at least
// 30000 bytes (in C a char is 1 byte (sizeof(char))
int initializeBrainfuckInterpreter(int memorySize){
    if(memorySize<DEFAULT_BF_MEMORY && verbose_mode){
        cout << "Warning! Brainfuck is intended to work with at least 30000 bytes of memory!\n";
    }

    bf_memory_size = memorySize;

    bf_memory = static_cast<unsigned char *>(malloc(memorySize * sizeof(char)));

    // Initialize the memory full of 0s
    for(int i=0;i<bf_memory_size;i++){
        // We should use setValueInBFMemory() since it is the safe way
        // But in this case we are sure this action will be safe
        // And this has better performance
        bf_memory[i] = 0;
    }

    setMemoryCursor(0);

    return 0; // No errors!
}

int setMemoryCursor(int newValue){
    if(newValue<0){
        if(debug_mode){
            cout << "[DEBUG] " << "Error: You can't set the memory cursor to less than 0!\n";
        }
        return -1;
    }else if(newValue>=getBFMemorySize()){
        if(debug_mode){
            cout << "[DEBUG] " << "Error: You can't set the memory cursor outside the Brainfuck memory!\n";
            cout << "[DEBUG] " << "Current Brainfuck memory is " << getBFMemorySize() << "\n";
            cout << "[DEBUG] " << "You can increase it with the -m and -i args\n";
        }
        return -2;
    }else{
        memory_cursor = newValue;
    }

    return 0;
}

int getMemoryCursor(){
    return memory_cursor;
}

int setValueInBFMemory(int cursor, int value){
    if(cursor<0 || cursor>=getBFMemorySize()){
        if(debug_mode)
            cout << "[DEBUG] "  << "Error: Trying to set value outside the Brainfuck memory!\n";
        return -1;
    }else if(value<0 || value > 255){
        if(debug_mode){
            cout << "[DEBUG] "  << "Error: Trying to set value greater than 1 byte!\n";
            cout << "[DEBUG] "  << "Value: " << value << "\n";
        }
        return -2;
    }else{
        bf_memory[cursor] = value;
    }

    return 0;
}

int setValueInCodeMemory(int cursor, int value){
    if(cursor<0 || cursor>=getCodeMemorySize()){
        if(debug_mode)
            cout << "[DEBUG] "  << "Error: Trying to set value outside the code memory!\n";
        return -1;
    }else if(value<0 || value > 255){
        if(debug_mode){
            cout << "[DEBUG] "  << "Error: Trying to set value greater than 1 byte!\n";
            cout << "[DEBUG] "  << "Value: " << value << "\n";
        }
        return -2;
    }else{
        code_memory[cursor] = value;
    }

    return 0;
}

char getValueInBFMemory(int cursor){
    if(cursor<0 || cursor>=getBFMemorySize()){
        if(debug_mode){
            cout << "[DEBUG] "  << "Error: Trying to get value outside the Brainfuck memory!\n";
            cout << "[DEBUG] "  << "Cursor: " << cursor << "\n";
        }

        return 0;
    }else{

        if(debug_mode){
            cout << "[DEBUG] "  << "Returning value " << bf_memory[cursor] << " from cursor " << cursor << "\n";
        }

        return bf_memory[cursor];
    }
}

char getValueInCodeMemory(int cursor){
    if(cursor<0 || cursor>=getCodeMemorySize()){
        if(debug_mode){
            cout << "[DEBUG] "  << "Error: Trying to get value outside the code memory!\n";
            cout << "[DEBUG] "  << "Cursor: " << cursor << "\n";
        }

        return 0;
    }else{
        if(debug_mode){
            cout << "[DEBUG] "  << "Returning value " << code_memory[cursor] << " from cursor " << cursor << "\n";
        }

        return code_memory[cursor];
    }
}

int getBFMemorySize(){
    return bf_memory_size;
}
int getCodeMemorySize(){
    return code_memory_size;
}

int dumpMemory(int size){
    if(size<0 || size>bf_memory_size){
        if(verbose_mode)
            cout << "Error: Invalid dump size!\n";
        return -1;
    }else{
        cout << "\n";
        for(int i=0;i<size;i++){
            if((i+1)%8==0 || i+1==size){
                cout << (static_cast<unsigned int>(bf_memory[i]) & 0xFF) << "\n";
            }else if(i==0 || (i+1)%8!=0 ){
                cout << (static_cast<unsigned int>(bf_memory[i]) & 0xFF) << " ";
            }
        }

        return 0;
    }
}