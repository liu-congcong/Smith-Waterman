#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GAP_OPEN -10
#define GAP_EXTENSION -10

typedef struct path
{
    char pair[2];
    struct path *next;
} PATH;

int string2integers(const char *, size_t, unsigned short *);
int get_source(int *, short *);
int get_path(size_t **, short **, const size_t, const size_t, const char *, const char *, PATH **, size_t *, size_t *, size_t *);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage:\n%s dna1 dna2.\n", argv[0]);
        exit(0);
    }

    size_t row_index, column_index, score, length;
    size_t region[4] = {0, 0, 0, 0};
    const size_t rows = strlen(argv[2]) + 1;
    const size_t columns = strlen(argv[1]) + 1;
    const int score_hash[5][5] = {
        5, -4, -4, -4, -4,
        -4, 5, -4, -4, -4,
        -4, -4, 5, -4, -4,
        -4, -4, -4, 5, -4,
        -4, -4, -4, -4, -4}; // ACGTN
    int scores[4];           // max score, left score, left & top score, top score
    short source;            // left -1, left & top 0, top 1

    unsigned short *row = malloc(sizeof(unsigned short) * columns);
    unsigned short *column = malloc(sizeof(unsigned short) * rows);

    string2integers(argv[1], columns - 1, row); // row string
    string2integers(argv[2], rows - 1, column); // column string

    // printf("rows: %lu\ncolumns: %lu\n", rows, columns);

    size_t **score_table = (size_t **)malloc(sizeof(size_t *) * rows);
    short **path_table = (short **)malloc(sizeof(short *) * rows);
    for (row_index = 0; row_index < rows; row_index++)
    {
        score_table[row_index] = malloc(sizeof(size_t) * columns);
        memset(score_table[row_index], 0, sizeof(size_t) * columns);
        path_table[row_index] = malloc(sizeof(short) * columns);
        memset(path_table[row_index], 0, sizeof(short) * columns);
    }

    for (row_index = 1; row_index < rows; row_index++)
    {
        for (column_index = 1; column_index < columns; column_index++)
        {
            scores[1] = score_table[row_index][column_index - 1] + (path_table[row_index][column_index - 1] ? GAP_EXTENSION : GAP_OPEN); // gap
            scores[2] = score_table[row_index - 1][column_index - 1] + score_hash[row[column_index]][column[row_index]];                 // match or mismatch
            scores[3] = score_table[row_index - 1][column_index] + (path_table[row_index - 1][column_index] ? GAP_EXTENSION : GAP_OPEN); // gap
            get_source(scores, &source);                                                                                                 // save max score to scores[0] and get source -1|0|1
            score_table[row_index][column_index] = scores[0];
            path_table[row_index][column_index] = source;
            // printf("scores: %f, %f, %f, max: %f, max: %f\n", scores[1], scores[2], scores[3], scores[0], score_table[row_index][column_index]);
        }
    }
    free(row);
    free(column);

    PATH *start_node = NULL;
    get_path(score_table, path_table, rows, columns, argv[1], argv[2], &start_node, &score, &length, region);
    for (row_index = 0; row_index < rows; row_index++)
    {
        free(score_table[row_index]);
        free(path_table[row_index]);
    }
    free(score_table);
    free(path_table);

    char *alignment_row = malloc(sizeof(char) * (length + 1));
    char *alignment_column = malloc(sizeof(char) * (length + 1));
    alignment_row[length] = 0;
    alignment_column[length] = 0;

    PATH *node = start_node;
    PATH *temp_node = start_node;
    for (size_t node_index = 0; node_index < length; node_index++)
    {
        alignment_row[node_index] = node->pair[0];
        alignment_column[node_index] = node->pair[1];
        node = node->next;
        free(temp_node);
        temp_node = node;
    }
    
    printf("alignment: %lu - %lu <-> %lu - %lu, length: %lu, score: %lu\n", region[0], region[1], region[2], region[3], length, score);
    puts(alignment_row);
    free(alignment_row);
    puts(alignment_column);
    free(alignment_column);
    return (0);
}

int string2integers(const char *input, size_t input_size, unsigned short *output)
{
    char temp;
    output[0] = 4;
    for (size_t index = 0; index < input_size; index++)
    {
        temp = input[index];
        if (temp == 'A' || temp == 'a')
            output[index + 1] = 0;
        else if (temp == 'C' || temp == 'c')
            output[index + 1] = 1;
        else if (temp == 'G' || temp == 'g')
            output[index + 1] = 2;
        else if (temp == 'T' || temp == 't')
            output[index + 1] = 3;
        else
            output[index + 1] = 4;
    }
    return (0);
}

int get_source(int *numbers, short *source)
{
    numbers[0] = 0;
    if (numbers[1] >= numbers[0] && numbers[1] >= numbers[3])
    {
        numbers[0] = numbers[1];
        *source = -1;
    }
    else if (numbers[3] >= numbers[0] && numbers[3] >= numbers[1])
    {
        numbers[0] = numbers[3];
        *source = 1;
    }
    if (numbers[2] >= numbers[0])
    {
        numbers[0] = numbers[2];
        *source = 0;
    }
    return (0);
}

int get_path(size_t **score_table, short **path_table, const size_t rows, const size_t columns, const char *row, const char *column, PATH **start_node, size_t *score, size_t *length, size_t *region)
{
    size_t local_score = 0, local_score_, row_index, column_index, score_row_index, score_column_index, local_length = 0;

    for (row_index = 0; row_index < rows; row_index++)
    {
        for (column_index = 0; column_index < columns; column_index++)
        {
            local_score_ = score_table[row_index][column_index];
            //printf("(%lu, %lu): %lu\n", row_index, column_index, local_score_);
            if (local_score_ >= local_score)
            {
                local_score = local_score_;
                score_row_index = row_index;
                score_column_index = column_index;
            }
        }
    }
    region[1] = score_column_index; // row string
    region[3] = score_row_index; // column string
    *score = local_score;
    // printf("(%lu, %lu): %f\n", score_row_index, score_column_index, score);

    PATH *previous_node = NULL;

    while (score_row_index && score_column_index && score_table[score_row_index][score_column_index])
    {
        region[0] = score_column_index; // row string
        region[2] = score_row_index; // column string
        PATH *node = malloc(sizeof(PATH));
        switch (path_table[score_row_index][score_column_index])
        {
        case -1:
        {
            score_column_index--;
            node->pair[0] = row[score_column_index];
            node->pair[1] = '-';
        }
        break;
        case 0:
        {
            score_row_index--;
            score_column_index--;
            node->pair[0] = row[score_column_index];
            node->pair[1] = column[score_row_index];
        }
        break;
        case 1:
        {
            score_row_index--;
            node->pair[0] = '-';
            node->pair[1] = column[score_row_index];
        }
        break;
        }
        local_length++;
        node->next = previous_node;
        previous_node = node;
    }
    *length = local_length;
    *start_node = previous_node;
    return (0);
}
