#include <sys/mman.h>
#include <iostream>
#include <fcntl.h>
#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include<stdbool.h>
#include <semaphore.h>


int create_tmp_file() {
    char filename[32] = {"tmp_file-XXXXXX"};
    int fd = mkstemp(filename);
    unlink(filename);
    if (fd < 1) {
        perror("Creating tmp file failed\n");
        exit(1);
    }
    write(fd, "#", 1);
    return fd;
}



int search(char* str1, char* str2, int a, int b)
{
    int flag = 0, count = 0, pos, i;
    if (a > b) {
        return -1;
    }
    for (i = 0; i < b - a + 1; i++) {
        if (str1[0] == str2[i]) {
            count++;
            for (int j = 0; j < a - 1; j++) {
                pos = i;
                if (str1[j] == str2[i + j]) {
                    flag = 1;
                } else {
                    flag = 0;
                    break;
                }
            } if (flag == 1) {
                break;
            }
        } 
    }
    if (flag == 1 && count > 0) {
        

        
        return i;

    }
    return -1;
}

int main() {
	int fd = create_tmp_file();

	char* shmem = (char*)mmap(NULL, 1000, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (shmem == MAP_FAILED) {
		perror("Mapping failed\n");
		exit(1);
	}

	sem_t* sem1 = sem_open("/sem1", O_CREAT, 777, 0);
	sem_t* sem2 = sem_open("/sem2", O_CREAT, 777, 0);
	if (sem1 == SEM_FAILED || sem2 == SEM_FAILED) {
		perror("Semaphore opening error\n");
		exit(1);
	}
	sem_unlink("/sem1");
	sem_unlink("/sem2");

	pid_t p = fork();
	if (p < 0) {
		perror("Fork error\n");
		exit(1);
	}
	if (p == 0) {
		char* patern;
		char* text;
		char ans[10];
		int len1, len2;
		patern = (char*)malloc(100 * sizeof(char));
		text = (char*)malloc(100 * sizeof(char));
		sem_wait(sem1);
		int i = 0;
		while(shmem[i] != '$') {
			patern[i] = shmem[i];
			i++;
		}
		len1 = i;
		i++;
		int j = 0;
		while (shmem[i] != '\0') {
			text[j] = shmem[i];
			j++;
			i++;

		}
		len2 = j;
		int pos = search(patern, text, len1, len2);
		if (pos == -1) {
            ans[0] = '-';
            ans[1] = '1';
            ans[2] = '\0';
        } else {
            if (pos >= 10) {
                ans[0] = (pos / 10) % 10 + '0';
                ans[1] = pos % 10 + '0';
                ans[2] = '\0';
            } else {
                ans[0] = pos + '0';
                ans[1] = '\0';

            }
        }
		memcpy(shmem + sizeof(char) * (strlen(patern) + strlen(text) + 1), ans, sizeof(char) * strlen(ans));
		free(patern);
		free(text);
		sem_post(sem2);
		sem_close(sem1);
		sem_close(sem2);
		close(fd);
		exit(EXIT_SUCCESS);
	} else if (p > 0) {
		char* str1;
		char* str2;
		char ans[10];
		str1 = (char*)malloc(100 * sizeof(char));
		str2 = (char*)malloc(100 * sizeof(char));
		scanf("%s %s", str1, str2);
		strcat(str1, "$");
		strcat(str1, str2);
		memcpy(shmem , str1, sizeof(char) * strlen(str1));
		sem_post(sem1);
		sem_wait(sem2);
		memcpy(ans, shmem + sizeof(char) * strlen(str1), sizeof(char) * 10);
		printf("%s\n", ans);
		free(str2);
		free(str1);

	}
	sem_close(sem1);
	sem_close(sem2);
	close(fd);




}