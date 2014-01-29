
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sox.h"

#define SONG_LENGTH_S 30

void usage();

void usage() {
   printf("usage: infile outfile\n");
}

void randomize_byte_order(unsigned int *sample_order) {
   unsigned int i;

   srand(103);

   for(i=0;i<SONG_LENGTH_S; i++) {
      sample_order[i] = rand()%30;
   }

}

int main(int argc, char* argv[])
{
   static sox_format_t *in_file, *out_file;
   sox_sample_t *buffer;
   size_t read;
   size_t sample_count;
   unsigned int sample_order[SONG_LENGTH_S];

   if(argc != 3) {
      usage();
      exit(EXIT_FAILURE);
   }

   if (sox_init() != SOX_SUCCESS) {
      fprintf(stderr, "error: could not initialize Sox\n");
      exit(EXIT_FAILURE);
   }

   if((in_file = sox_open_read(argv[1], NULL, NULL, NULL)) == NULL) {
      fprintf(stderr, "error could not read input file\n");
      exit(EXIT_FAILURE);
   }

   if((out_file = sox_open_write(argv[2], &in_file->signal,
                     NULL, "wav", NULL, NULL)) == NULL) {
      fprintf(stderr, "error could not open output file\n");
      exit(EXIT_FAILURE);
   }


   sample_count = SONG_LENGTH_S * in_file->signal.rate * in_file->signal.channels;

   buffer = (sox_sample_t *) malloc(sizeof(sox_sample_t) * sample_count);

   randomize_byte_order(sample_order);

   if (sox_read(in_file, buffer, sample_count) != sample_count) {
      fprintf(stderr, "Incorrect number of samples read");
   }

   unsigned int i=0;

   for(i=0;i<SONG_LENGTH_S;i++) {
      sox_write(out_file, buffer + (sample_order[i] * (((unsigned int)(in_file->signal.rate)
                                                    * in_file->signal.channels))),
                                                    in_file->signal.rate * in_file->signal.channels);
   }

   free(buffer);

   sox_close(in_file);
   sox_close(out_file);


   sox_quit();



   return 0;

}
