#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void print_error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}


int main (int argc, char *argv[]){

FILE * input;
int interactive;
char *line = NULL;
size_t len =0;
ssize_t nread;


if(argc==1){
    interactive=1;
}else if(argc==2){
    interactive=0;//batch mode
}else{
    print_error();
    exit(1);
}

input = stdin;
if(argc==2){
    input=fopen(argv[1],"r");
    if(input==NULL){
        print_error();
        exit(1);
    }
}

while(1){
if(interactive){
    printf("chill> ");
    fflush(stdout);
}

nread = getline(&line, &len, input);

if (nread == -1){
    exit(0);
}

line[strcspn(line, "\n")] = '\0'; //delete white space

if (strcmp(line, "exit") == 0) { //compare
    exit(0);
}


}
return 0;
}
