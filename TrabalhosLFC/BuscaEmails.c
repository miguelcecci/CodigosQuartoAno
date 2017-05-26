#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#define MAX_ERROR_MSG 0x1000

int arqcont=0;
long mailcount=0;
char sizeText[1024];
const char *regex_text;
const char *find_text;
regex_t r;

static void lookup(){
    DIR *dirpont;
    struct dirent *dp;

    if ((dirpont = opendir(".")) == NULL) {
        perror("nao pode abrir '.'");
        return;
    }


    do {
        errno = 0;
        if ((dp = readdir(dirpont)) != NULL) {
          if(dp->d_type == 4 && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0 && (strpbrk(dp->d_name, ".") == NULL)){
              chdir(dp->d_name);
              lookup();
              chdir("..");
          }else{
            if(dp->d_type == 8){
              //aqui vai a busca -----------------------------------------------
              FILE *temparq = fopen(dp->d_name, "r");
              if(!temparq){
                printf("nao foi possivel abrir o arquivo\n");
              }
              while(fscanf(temparq, "%s", sizeText) != EOF){
                match_regex(& r, sizeText);
              }
              fclose(temparq);
              arqcont++;
            }
          }

        }
    } while (dp != NULL);


    if (errno != 0){
      perror("Erro lendo o diretorio");
    }

    (void) closedir(dirpont);
    return;
}

int match_regex (regex_t * r, const char * to_match){
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 10;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    while (1) {
        int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) {
            return nomatch;
        }
        mailcount++;
        int start;
        int finish;
        for (i = 0; i < n_matches; i++) {
            if (m[i].rm_so == -1) {
                break;
            }
            start = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);

        }
        printf ("'%.*s'\n", (finish - start), to_match);
        p += m[0].rm_eo;
    }
    return 0;
}

static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
	char error_message[MAX_ERROR_MSG];
	regerror (status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    regex_text = "([a-z]*?[\\.]?[a-z]*@[a-z]*\\.[a-z]*?[\\.]?[a-z]*)";
    compile_regex (& r, regex_text);
    lookup();
    printf("%d arquivos verificados no total, %ld endereços de email encontrados.\n", arqcont, mailcount);
    regfree(& r);
    return (0);
}
