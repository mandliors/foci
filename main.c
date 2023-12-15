#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Compile-time constants
enum {
    TEAM_NAME_MAX_LENGTH = 20,
    MAX_MATCH_COUNT      = 12
};

typedef double Vector4[4];
typedef double Matrix4[4][4];

typedef char Team[TEAM_NAME_MAX_LENGTH + 1];
typedef struct Match {
    Team t1_name, t2_name;
    unsigned t1_score, t2_score;
} Match;

bool read_matches(Match* matches, const char* path);
int  find_teams(Team* teams, Match* matches);
int  find_team_index(Team* teams, const char* team_name);
bool build_rank_matrix(Matrix4 matrix, Team* teams, Match* matches);
void matrix4_find_eigen(Matrix4 matrix, double* eigenvalue, Vector4 eigenvector);
void eigen_to_percentages(Vector4 eigenvector, Vector4 percentages);

int main(void)
{
    // Process the matches
    Match matches[MAX_MATCH_COUNT];
    Team teams[4] = { "" };
    Matrix4 rank_matrix = { 0.0 };

    if (!read_matches(matches, "matches.txt")) return -1;
    if (find_teams(teams, matches) < 4) return -1;
    if (!build_rank_matrix(rank_matrix, teams, matches)) return -1;

    // Find an eigenvalue and an eigenvector, then calculate percentages
    double eigenvalue;
    Vector4 eigenvector;
    Vector4 percentages;
    
    matrix4_find_eigen(rank_matrix, &eigenvalue, eigenvector);
    eigen_to_percentages(eigenvector, percentages);
    for (int i = 0; i < 4; i++)
        printf("%s: %.2f\%\n", teams[i], percentages[i]);

    return 0;
}

bool read_matches(Match* matches, const char* path)
{
    FILE* fp = fopen(path, "r");
    if (fp == NULL)
    {
        printf("error opening file %s\n", path);
        return false;
    }
     
    int i = 0;
    while (fscanf(fp, "%[^-]-%[^ ] %u-%u", matches[i].t1_name, matches[i].t2_name, &matches[i].t1_score, &matches[i].t2_score) == 4)
    {
        fgetc(fp); // read the newline character
        i++;
    }
    
    if (i < MAX_MATCH_COUNT)
        matches[i].t1_name[0] = '\0';

    fclose(fp);
    return true;
}

int find_teams(Team* teams, Match* matches)
{
    int team_count = 0;
    for (int i = 0; i < MAX_MATCH_COUNT && matches[i].t1_name[0] != '\0'; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (strcmp(matches[i].t1_name, teams[j]) == 0)
                break;
            else if (teams[j][0] == '\0')
            {
                strcpy(teams[j], matches[i].t1_name);
                team_count++;
                break;
            }
        }
        for (int j = 0; j < 4; j++)
        {
            if (strcmp(matches[i].t2_name, teams[j]) == 0)
                break;
            else if (teams[j][0] == '\0')
            {
                strcpy(teams[j], matches[i].t2_name);
                team_count++;
                break;
            }
        }
        if (team_count == 4)
            break;
    }
    return team_count;
}

int find_team_index(Team* teams, const char* team_name)
{
    for (int i = 0; i < 4; i++)
        if (strcmp(teams[i], team_name) == 0)
            return i;
    return -1;
}

bool build_rank_matrix(Matrix4 matrix, Team* teams, Match* matches)
{
    for (int i = 0; i < MAX_MATCH_COUNT && matches[i].t1_name[0] != '\0'; i++)
    {
        int t1 = find_team_index(teams, matches[i].t1_name);
        int t2 = find_team_index(teams, matches[i].t2_name);
        if (t1 == -1 || t2 == -1) return false;

        if (matches[i].t1_score > matches[i].t2_score)
            matrix[t1][t2] += 3.0;
        else if (matches[i].t1_score < matches[i].t2_score)
            matrix[t2][t1] += 3.0;
        else
        {
            matrix[t1][t2] += 1.0;
            matrix[t2][t1] += 1.0;
        }
    }
    return true;
}

void matrix4_find_eigen(Matrix4 matrix, double* eigenvalue, Vector4 eigenvector)
{
    double epsilon = 1.0e-10;
    int max_iterations = 1000;

    // Initial guess for the eigenvector
    double v[4] = {1.0, 1.0, 1.0, 1.0};
    double lambda = 0.0;

    for (int iter = 0; iter < max_iterations; ++iter)
    {
        double v_new[4] = {0.0};

        // Matrix-vector multiplication: Av
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                v_new[i] += matrix[i][j] * v[j];

        // Find the magnitude of the resulting vector
        double magnitude = 0.0;
        for (int i = 0; i < 4; ++i)
            magnitude += v_new[i] * v_new[i];
        magnitude = sqrt(magnitude);

        // Normalize the vector
        for (int i = 0; i < 4; ++i)
            v[i] = v_new[i] / magnitude;

        // Compute the eigenvalue
        double lambda_new = 0.0;
        for (int i = 0; i < 4; ++i)
        {
            double Av_i = 0.0;
            for (int j = 0; j < 4; ++j) 
                Av_i += matrix[i][j] * v[j];
            lambda_new += v[i] * Av_i;
        }

        // Check for convergence
        if (fabs(lambda_new - lambda) < epsilon)
            break;

        lambda = lambda_new;
    }

    // Assign the computed eigenvalue and eigenvector
    *eigenvalue = lambda;
    for (int i = 0; i < 4; ++i)
        eigenvector[i] = v[i];
}

void eigen_to_percentages(Vector4 eigenvector, Vector4 percentages)
{
    double sum = 0.0;
    for (int i = 0; i < 4; i++)
        sum += eigenvector[i];
    for (int i = 0; i < 4; i++)
        percentages[i] = eigenvector[i] / sum * 100.0;
}