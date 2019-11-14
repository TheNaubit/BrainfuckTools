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
int cursor; // Current in-memory position

int debug_mode = 0;
int verbose_mode = 0;
int size_memory_dump = DEFAULT_DUMP_MEMORY;
int size_brainfuck_memory = DEFAULT_BF_MEMORY;

int parseArgs(int argc, char** argv);
string cleanCode(string code);
int initializeBrainfuckInterpreter(int memorySize);
int interprete(int code_cursor, char order);
int setValueInBFMemory(int cursor, int value);
char getValueInBFMemory(int cursor);
int dumpMemory(int size);
int showHelp();

int showHelp(){
    printf("--- Brainfuck Interpreter (bit) v.%d ---\n",BF_INTERPRETER_VERSION);
    printf("This tool has two modes: by arguments and interactive.\n");
    printf("If you don't enter arguments or if you don't enter Brainfuck code in\n");
    printf("the arguments (with -t or -f), the interactive mode will run.\n\n");
    printf("Arguments:\n");
    printf("-h: It shows this help.\n");
    printf("-v: Verbose mode, it shows all the program outputs (except debug output if it is not enabled).\n");
    printf("-t CODE: It runs the Brainfuck code. You have to enter the CODE without spaces nor newlines in this mode.\n");
    printf("-f PATH_TO_FILE: It runs the Brainfuck code inside a file.\n");
    printf("-m INT: It sets the size of the Brainfuck interpreters memory. Not recommended less than 30000. (It represents 30000 bytes)\n");
    printf("-s INT: It sets the size of the memory dumps (in debug mode). By default 32.\n");
    printf("-o: (Not implemented yet!) It optimizes the code before running it. For big Brainfuck programs this option is recommended.\n\n");
    printf("For any other question or issue check the GitHub repo: https://github.com/NauCode/BrainfuckTools\n");
    return 0;
}

// Valid args:
// -d: Debug mode enabled
// -v: Verbose mode, it shows program requests.
// -h: It shows the help (not compatible with other args)
// -t {text without new line characters except at the end and no spaces}: It runs a brainfuck code
// -f {path to file with brainfuck code}: It runs the brainfuck code inside a file (not compatible with -t arg)
// -m {int}: It sets the size of the memory of the Brainfuck interpreter. Not recommended less than 30000
// -s {int}: It sets the size of the memory dumps (in debug mode). By default 40.

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
                    printf("Error: You already set the code source!\n");
                    return -3;
                }
                next_mustnt_arg=1;
                next_mustnt_arg_from='t';
                break;
            case 'f':
                if(code!=""){
                    printf("Error: You already set the code source!\n");
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
                    printf("Error: Invalid args!\n");
                    return -4;
                }
                break;

        }
    }

    if(code==""){
        if(argc>1){
            printf("No code entered! Use -t or -f next time!\n");
            return -1;
        }else{
            return -2;
        }
    }

    return 0;
}

int main(int argc, char** argv) {

    string code = "";

    // Since we have args (remember we always have argc>=1 since argv[0] is the
    // ./{executable_file}), we have to use them!

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
            printf("Error: We had some error initializing Brainfuck interpreter!\n");
        }
        // If we had some error initializing it, we can't run the Brainfuck programs
        // So we exit with error -2
        return -2;
        // exit(-2); As you prefer
    }else{
        // I am not sure if this should go as verbose or debug
        if(verbose_mode) {
            printf("Brainfuck interpreter initialized!\n");
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
    while(code_cursor>=0 && code_cursor<code_memory_size){
        code_cursor = interprete(code_cursor, code_memory[code_cursor]);
    }

    // If everything went fine we can end the program with no errors
    return 0;
    // exit(0); ?
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

int interprete(int code_cursor, char order){
    int user_input;
    int code_cursor_ret = code_cursor+1; // By default returns code_cursor + 1

    switch(order){
        case '>':
            if(debug_mode){
                cout << ">: Moving from " << cursor << " to " << cursor+1 << "\n";
            }
            cursor+=1;
            break;
        case '<':
            if(debug_mode){
                cout << "<: Moving from " << cursor << " to " << cursor-1 << "\n";
            }
            cursor-=1;
            break;
        case '+':
            if(debug_mode){
                cout << "+: Adding 1 at " << cursor << " from " << (static_cast<unsigned int>(getValueInBFMemory(cursor)) & 0xFF) << " to " << getValueInBFMemory(cursor)+1 << "\n";
            }
            setValueInBFMemory(cursor, getValueInBFMemory(cursor)+1);
            break;
        case '-':
            if(debug_mode){
                cout << "-: Removing 1 at " << cursor << " from " << (static_cast<unsigned int>(getValueInBFMemory(cursor)) & 0xFF) << " to " << getValueInBFMemory(cursor)-1 << "\n";
            }
            setValueInBFMemory(cursor, getValueInBFMemory(cursor)-1);
            break;
        case '.':
            if(debug_mode){
                cout << ".: Printing value at " << cursor << ": int()" << (static_cast<unsigned int>(getValueInBFMemory(cursor)) & 0xFF) << " char():"<<getValueInBFMemory(cursor)<<"\n";
            }
            cout << getValueInBFMemory(cursor);
            break;
        case ',':
            // Important the space before %c!!!
            //scanf(" %c",&user_input);
            user_input=getchar();
            setValueInBFMemory(cursor, user_input);
            if(debug_mode){
                cout << ",: Getting input: " << user_input << " and saving at " << cursor << "\n";
            }
            break;
        case '[':
            if(bf_memory[cursor]==0){
                int tmp_cursor = code_cursor+1;
                int close_bracket_found = 0;
                int current_depth = 0;
                while(code_cursor<code_memory_size && close_bracket_found == 0){
                    char current_tmp_char = code_memory[tmp_cursor];
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
                    cout << "[: Since memory at " << cursor << " is 0, we jump to " << tmp_cursor+1 << " in code memory\n";
                }
                code_cursor_ret = tmp_cursor+1;
            }else{
                if(debug_mode){
                    cout << "[: Since memory at " << cursor << " is not 0, we do nothing!\n";
                }
            }
            break;
        case ']':
            if(bf_memory[cursor]!=0){
                int tmp_cursor = code_cursor-1;
                int open_bracket_found = 0;
                int current_depth = 0;
                while(tmp_cursor>=0 && open_bracket_found == 0){
                    char current_tmp_char = code_memory[tmp_cursor];
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
                    cout << "]: Since memory at " << cursor << " is not 0, we jump to " << tmp_cursor+1 << " in code memory\n";
                }
                code_cursor_ret = tmp_cursor+1;
            }else{
                if(debug_mode){
                    cout << "]: Since memory at " << cursor << " is 0, we do nothing!\n";
                }
            }
            break;
        default:
            if(debug_mode){
                cout << "Not valid instruction found, skipping...\n";
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
    if(memorySize<DEFAULT_BF_MEMORY){
        cout << "Warning! Brainfuck is intended to work with at least 30000 bytes of memory!\n";
    }

    bf_memory_size = memorySize;

    bf_memory = static_cast<unsigned char *>(malloc(memorySize * sizeof(char)));

    // Initialize the memory full of 0s
    for(int i=0;i<bf_memory_size;i++){
        bf_memory[i] = 0;
    }

    cursor = 0;

    return 0; // No errors!
}

int setValueInBFMemory(int cursor, int value){
    if(cursor<0 || cursor>=bf_memory_size){
        // cout << "Error: Trying to set value outside on Brainfuck memory!\n";
        return -1;
    }else if(value<0 || value > 255){
        /*if(DEBUG_ENABLED){
            cout << "Error: Trying to set value greater than 1 byte!\n";
            cout << "Value: " << value << "\n";
        }*/
        return -2;
    }else{
        bf_memory[cursor] = value;
    }

    return 0;
}

char getValueInBFMemory(int cursor){
    if(cursor<0 || cursor>=bf_memory_size){
        /*if(DEBUG_ENABLED){
            cout << "Error: Trying to get value outside on Brainfuck memory!\n";
            cout << "Cursor: " << cursor << "\n";
        }*/

        return 0;
    }else{

        /*if(DEBUG_ENABLED){
            cout << "Returning value " << bf_memory[cursor] << " from cursor " << cursor << "\n";
        }*/

        return bf_memory[cursor];
    }
}

int dumpMemory(int size){
    if(size<0 || size>bf_memory_size){
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