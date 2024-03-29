/**
 * Projec : gtsp (voyageur de commerce)
 *
 * Date   : 07/04/2014
 * Author : Olivier Grunder
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define NBR_TOWNS 10

/* Distance matrix */
double dist[NBR_TOWNS][NBR_TOWNS] ;

/* Each edge has a starting and ending node */
int starting_town[NBR_TOWNS] ;
int ending_town[NBR_TOWNS] ;

/* no comment */
int best_solution[NBR_TOWNS] ;
double best_eval=-1.0 ;


/**
 * Berlin52 :
 *  6 towns : Best solution (2315.15): 0 1 2 3 5 4
 * 10 towns : Best solution (2826.50): 0 1 6 2 7 8 9 3 5 4
 */
float coord[NBR_TOWNS][2];

//Function to retrieve coordinates from berlin52.tsp

void read_file()
{
    FILE* file = fopen("berlin52.tsp", "r");
    if(file == NULL)
    {
        perror("Error file opening");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    char temp[100];
    while(strcmp(temp, "NODE_COORD_SECTION") != 0)
    {
        fscanf(file, "%s", temp);
    }

    //Fill the array with the data from the file
    for(int j = 0; j < NBR_TOWNS; j++)
    {
        int temp_nbr;
        fscanf(file, "%d", &temp_nbr);
        fscanf(file, "%f", &coord[j][0]);
        fscanf(file, "%f", &coord[j][1]);

        fgets(temp, 100, file);
    }
    fclose(file);
}


void print_matrix(double d[NBR_TOWNS][NBR_TOWNS])
{
    int i, j ;
    for (i=0; i<NBR_TOWNS; i++)
    {
        printf ("%d:", i) ;
        for (j=0; j<NBR_TOWNS; j++)
        {
            printf ("%6.1f ", d[i][j]) ;
        }
        printf ("\n") ;
    }
}


void print_solution(int* sol, double eval)
{
    int i ;
    printf ("(%.2f): ", eval) ;
    for (i=0; i<NBR_TOWNS; i++)
        printf ("%d ",sol[i]);
    printf("\n\n") ;
}


double evaluation_solution(int* sol)
{
    double eval=0 ;
    int i ;
    for (i=0; i<NBR_TOWNS-1; i++)
    {
        eval += dist[sol[i]][sol[i+1]] ;
    }
    eval += dist[sol[NBR_TOWNS-1]][sol[0]] ;

    return eval ;

}

bool valueinarray(int val, int*arr)
{
    for(int i = 0; i < NBR_TOWNS; i++)
    {
        if(arr[i] == val)
            return true;
    }
    return false;
}


double build_nearest_neighbour()
{
    /* solution of the nearest neighbour */
    int i, sol[NBR_TOWNS] ;

    /* evaluation of the solution */
    double eval = 0 ;

    sol[0] = 0 ;

    int ville;
    int nextville = 0;
    double min;

    for (int k=1; k<NBR_TOWNS; k++)
    {
        ville = nextville;
        min = 99999;
        for (int j=0; j<NBR_TOWNS-1; j++)
        {
            bool v = valueinarray(j+1, sol);
            if (!v && j+1 != ville)
            {
                if (dist[ville][j+1] < min)
                {
                    nextville = j+1;
                    min = dist[ville][j+1];
                }
            }
            else
            {
            }
        }
        sol[k]=nextville;
    }

    eval = evaluation_solution(sol) ;
    printf("Nearest neighbour ") ;
    print_solution (sol, eval) ;
    for (i=0;i<NBR_TOWNS;i++) best_solution[i] = sol[i] ;
    best_eval  = eval ;

    return eval ;
}


void build_solution()
{
    int i, solution[NBR_TOWNS] ;

    int indiceCour = 0;
    int villeCour = 0;

    while (indiceCour < NBR_TOWNS)
    {

        solution[indiceCour] = villeCour ;

        // Test si le cycle est hamiltonien
        for (i = 0; i < indiceCour; i++)
        {
            if (solution[i] == villeCour)
            {
                /* printf ("cycle non hamiltonien\n") ; */
                return;
            }
        }
        // Recherche de la ville suivante
        int trouve = 0;
        int i = 0;
        while ((!trouve) && (i < NBR_TOWNS))
        {
            if (starting_town[i] == villeCour)
            {
                trouve = 1;
                villeCour = ending_town[i];
            }
            i++;
        }
        indiceCour++;
    }

    double eval = evaluation_solution(solution) ;

    if (best_eval<0 || eval < best_eval)
    {
        best_eval = eval ;
        for (i=0; i<NBR_TOWNS; i++)
            best_solution[i] = solution[i] ;
        printf("New best solution: ") ;
        print_solution (solution, best_eval) ;
    }
    return;
}

double count_regrets(double d0[NBR_TOWNS][NBR_TOWNS], int x, int y)
{
    double min_row = LONG_MAX;
    double min_col = LONG_MAX;

    for (int t=0; t<NBR_TOWNS; t++)
    {
        // boucle sur les colonnes qui n'ont pas été traitées
        if (y != t && d0[x][t] >= 0)
        {
            if (d0[x][t] < min_row)
                min_row = d0[x][t];
        }

        if (min_row == LONG_MAX)
            min_row = 0.0;

        // boucle sur les lignes qui n'ont pas été traitées
        if (x != t && d0[t][y] >= 0)
        {
            if (d0[t][y] < min_col)
                min_col = d0[t][y];
        }

        if (min_col == LONG_MAX)
            min_col = 0.0;
    }
    return min_col + min_row;
}

double reduct_matrix(double d0[NBR_TOWNS][NBR_TOWNS], int type)
{
    double total = 0;
    for (int i = 0; i < NBR_TOWNS; i++)
    {
        double min = LONG_MAX;
        for (int j = 0; j < NBR_TOWNS; j++)
        {
            if (type == 0)
            {
                // Si la valeur vaut -1, c'est qu'on l'a déjà traité/ on est sur une diagonale
                if (d0[i][j] != -1 && d0[i][j] < min)
                    min = d0[i][j];
            }
            else
            {
                if (d0[j][i] != -1 && d0[j][i] < min)
                    min = d0[j][i];
            }
        }

        // Si le minimum est 0, on n'a rien à faire

        if (min != LONG_MAX)
        {
            for (int j = 0; j < NBR_TOWNS; j++)
            {
                if (type == 0 && d0[i][j] != -1)
                    d0[i][j] = d0[i][j] - min;
                else if (type == 1 && d0[j][i] != -1)
                    d0[j][i] = d0[j][i] - min;
            }

            total += min;
        }
    }
    return total;
}

void little_algorithm(double d0[NBR_TOWNS][NBR_TOWNS], int iteration, double eval_node_parent)
{

    if (iteration == NBR_TOWNS)
    {
        build_solution ();
        return;
    }

    /* Do the modification on a copy of the distance matrix */
    double d[NBR_TOWNS][NBR_TOWNS] ;
    memcpy (d, d0, NBR_TOWNS*NBR_TOWNS*sizeof(double)) ;

    double eval_node_child = eval_node_parent;

    /* On place un 0 sur chaque ligne et colonne de la matrice et on récupère la somme des minimums enlevés*/
    /* on met a jour l'évaluation du noeud "enfant" avec le total récupéré*/
    eval_node_child += reduct_matrix(d, 0);
    eval_node_child += reduct_matrix(d, 1);

    /* Cut : stop the exploration of this node */
    if (best_eval>=0 && eval_node_child >= best_eval)
        return;


    /**
     *  Compute the penalties to identify the zero with max penalty
     *  If no zero in the matrix, then return, solution infeasible
     */

    int izero=-1, jzero=-1;
    double maxpen = -1;

    for (int i=0; i<NBR_TOWNS; i++)
    {
        for (int j=0; j<NBR_TOWNS; j++)
        {
            if (d[i][j]==0)
            {
                double pen = count_regrets(d, i, j);
                if (pen > maxpen)
                {
                    maxpen = pen;
                    izero = i;
                    jzero = j;
                }
            }
        }
    }

    /**
     *  Store the row and column of the zero with max penalty in
     *  starting_town and ending_town
    */
    starting_town[iteration] = izero;
    ending_town[iteration] = jzero;

    /** LITTLE+ OPTIMIZATION */
    if (jzero == 0 && izero == ending_town[iteration - 1])
        return;

    if (maxpen == -1)
        return;

    /* Do the modification on a copy of the distance matrix */
    double d2[NBR_TOWNS][NBR_TOWNS] ;
    memcpy (d2, d, NBR_TOWNS*NBR_TOWNS*sizeof(double)) ;

    /**
     *  Modify the matrix d2 according to the choice of the zero with the max penalty
     */

    for (int t=0; t<NBR_TOWNS; t++)
    {
        d2[izero][t] = -1;
        d2[t][jzero] = -1;
    }

    d2[jzero][izero] = -1;

    /** Explore left child node according to given choice */
    little_algorithm(d2, iteration + 1, eval_node_child);

    /** Do the modification on a copy of the distance matrix */
    memcpy (d2, d, NBR_TOWNS*NBR_TOWNS*sizeof(double));

    /**
     *  Modify the dist matrix to explore the other possibility : the non-choice
     *  of the zero with the max penalty
     */

    d2[izero][jzero] = -1;

    /** Explore right child node according to non-choice */
    little_algorithm(d2, iteration, eval_node_child);

}


int main (int argc, char* argv[])
{


    // On remplit le tableau des coordonnées à partir du fichier berlin52.tsp
    read_file();

    best_eval = -1 ;

    /* Print problem informations */
    printf ("Points coordinates:\n") ;
    int i, j;
    for (i=0; i<NBR_TOWNS; i++)
    {
        printf ("Node %d: x=%f, y=%f\n", i, coord[i][0], coord[i][1]) ;
    }
    printf ("\n") ;


    /* Calcul de la matrice des distances */

    for (i=0; i<NBR_TOWNS; i++)
    {
        for (j=0; j<NBR_TOWNS; j++)
        {
            if (i==j)
                dist[i][j]=-1;
            else
            {
                dist[i][j] = sqrt(pow(coord[j][0]-coord[i][0],2) + pow(coord[j][1]-coord[i][1],2));
            }
        }
    }

    printf ("Distance Matrix:\n") ;
    print_matrix (dist) ;
    printf ("\n") ;

    //Nearest Neighbour heuristic

    double nearest_neighbour = build_nearest_neighbour() ;

    // Little

    clock_t start, end;
    double time_spent;
    start = clock();

    int iteration = 0 ;
    double lowerbound = 0.0;
    little_algorithm(dist, iteration, lowerbound);
    end = clock();

    printf("\n\nBest solution:") ;
    print_solution (best_solution, best_eval) ;

    time_spent = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("\n\nExecution time : %f\n\n", time_spent);
    printf ("Hit RETURN!\n") ;
    getchar() ;

    return 0 ;
}
