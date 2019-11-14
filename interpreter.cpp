#include <iostream>
#include <stdio.h>
#include <bits/stdc++.h>
#include <string.h>
#include <argp.h>
#include <stdbool.h>

#define DEFAULT_BF_MEMORY 30000

using namespace std;

unsigned char* bf_memory; // Stored as a pointer to dinamically initialize it
unsigned char* code_memory;
int bf_memory_size;
int code_memory_size;
int cursor; // Current in-memory position

int debug_mode = 0;
int verbose_mode = 0;
int size_memory_dump = 40;
int size_brainfuck_memory = DEFAULT_BF_MEMORY;

char doc[] = "A program which runs Brainfuck code with several extra options.";
static struct argp_option options[] = {
        {"verbose", 'v', 0, 0, "Verbose mode, it shows all the program outputs (except debug output if not enabled)"},
        {"debug", 'd', 0, 0, "Debug mode, it shows all the debug outputs (including explanations of every action and memory dumps)"},
        {"text", 't', "TEXT", 0, "It runs a brainfuck code, do not include new lines on it (except the final one)"},
        {"file", 'f', "FILE", 0, "It runs a brainfuck code from a file (not compatible with -t arg)"},
        {"memorySize", 'm', "SIZE", 0, "It sets the size of the memory of the Brainfuck interpreter. Not recommended less than 30000"},
        {"dumpSize", 's', "SIZE", 0, "It sets the size of the memory dumps (in debug mode). By default 40"},
        { 0 }
};
struct arguments {
    bool verbose;
    bool debug;
    char *text;
    char *file;
    int *memorySize;
    int *dumpSize;
};

int parseArgs(int argc, char** argv);
string optimizeCode(string code);
int initializeBrainfuckInterpreter(int memorySize);
int interprete(int code_cursor, char order);
int setValueInBFMemory(int cursor, int value);
char getValueInBFMemory(int cursor);
int dumpMemory(int size);

// Valid args:
// -d: Debug mode enabled
// -v: Verbose mode, it shows program requests.
// -h: It shows the help (not compatible with other args)
// -t {text without new line characters except at the end and no spaces}: It runs a brainfuck code
// -f {path to file with brainfuck code}: It runs the brainfuck code inside a file (not compatible with -t arg)
// -m {int}: It sets the size of the memory of the Brainfuck interpreter. Not recommended less than 30000
// -s {int}: It sets the size of the memory dumps (in debug mode). By default 40.

int parseArgs(int argc, char** argv){
    string code = "";
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
                break;
            default:
                if(next_mustnt_arg==1){
                    switch(next_mustnt_arg_from){
                        case 't':
                            code = optimizeCode(argv[argi]);
                            code_memory_size = code.length();
                            code_memory = static_cast<unsigned char *>(malloc(code_memory_size * sizeof(char)));

                            for(int i=0; i<code_memory_size; i++){
                                code_memory[i] = code.at(i);
                            }
                            break;
                        case 'f':
                            // Load the file and parse the code
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
                    return -2;
                }
                break;

        }
    }

    if(code==""){
        printf("No code entered! Use -t or -f next time!\n");
        return -1;
    }

    return 0;
}

int main(int argc, char** argv) {

    string code = "";

    // Since we have args (remember we always have argc>=1 since argv[0] is the
    // ./{executable_file}), we have to use them!
    if(argc>1){
        // Let's loop the args
        for(int i = 1; i < argc; ++i){
            cout << argv[i] << "\n";
        }
        parseArgs(argc,argv);
    }else {
        //code = "++>+++++[<+>-]++++++++[<++++++>-]<.";
        cout << "Set the code you want to run:\n";
        // Reads input with spaces!
        getline(cin >> ws, code); //cin >> code;

        code = optimizeCode(code);

        code_memory_size = code.length();

        code_memory = static_cast<unsigned char *>(malloc(code_memory_size * sizeof(char)));

        for(int i=0; i<code_memory_size; i++){
            code_memory[i] = code.at(i);
        }
    }

    if(initializeBrainfuckInterpreter(size_brainfuck_memory)!=0){
        if(debug_mode) {
            cout << "We had some error initializing Brainfuck interpreter!\n";
        }
        return -1;
    }else{
        if(debug_mode)
            cout << "Brainfuck interpreter initialized!\n";
    }

    if(debug_mode){
        dumpMemory(size_memory_dump);
    }

    int code_cursor = 0;
    while(code_cursor>=0 && code_cursor<code_memory_size){
        code_cursor = interprete(code_cursor, code_memory[code_cursor]);
    }


    return 0;
}

string optimizeCode(string code){
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