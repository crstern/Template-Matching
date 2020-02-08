#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct pixel{
    unsigned char b, g, r;
};


typedef struct detectie{
    int i, j;
    int culoare;
    double cor;
};


extern struct pixel * incarca_imagine(char *numeImagine)
{
    unsigned int latime_img;
    unsigned int inaltime_img;
    FILE *img = fopen(numeImagine, "rb");
    if(img == NULL)
    {
        printf("Eroare la deschiderea imaginii");
        return NULL;
    }
    fseek(img, 18, SEEK_SET);
    fread(&latime_img, sizeof(int), 1, img);
    fread(&inaltime_img, sizeof(int), 1, img);

    int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;
    //printf("padding = %d \n",padding);

    struct pixel *Lin = (struct pixel *)malloc((latime_img + padding) * inaltime_img * sizeof(struct pixel));
    fseek(img, 54, SEEK_SET);
    for(int i = inaltime_img - 1;i >= 0;i--){
        for(int j = 0;j < latime_img;j++)
        {
            unsigned char aux[3];
            fread(aux, sizeof(unsigned char), 3, img);
            Lin[i * latime_img + j].b = aux[0];
            Lin[i * latime_img + j].g = aux[1];
            Lin[i * latime_img + j].r = aux[2];
        }
        fseek(img, padding, SEEK_CUR);
    }

    fclose(img);
    return Lin;
}

extern void salveaza_imagine(char *numeImagine)
{
    FILE *img = fopen(numeImagine, "rb");
    FILE *imgOUT = fopen("savedImg.bmp", "wb");
    if(img == NULL)
    {
        printf("Eroare la deschiderea imaginii din care citesc\n");
        return;
    }

    if(imgOUT == NULL)
    {
        printf("Eroare la crearea imaginii\n");
        return;
    }
    int latime_img, inaltime_img;
    fseek(img, 18, SEEK_SET);
    fread(&latime_img, sizeof(int), 1, img);
    fread(&inaltime_img, sizeof(int), 1, img);
    fseek(img, 0, SEEK_SET);
    struct pixel *lin = incarca_imagine(numeImagine);


    char c;
    for(;fread(&c, 1, 1, img);)
    {
        fwrite(&c, 1, 1, imgOUT);
        fflush(imgOUT);
    }
    int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;
    printf("padding = %d \n",padding);

    fseek(imgOUT, 54, SEEK_SET);
    for(int i = inaltime_img - 1;i >= 0;i--)
    {
        for(int j = 0;j < latime_img;j++)
        {
            fwrite(lin + (i * latime_img + j), sizeof(struct pixel), 1, imgOUT);
        }
        fseek(imgOUT, padding, SEEK_CUR);

    }
    fclose(img);
    fclose(imgOUT);
    free(lin);
}

void test_criptare(char *nume_imagine, FILE *out)
{
    unsigned int latime_img, inaltime_img, i, j;
    float valR, valB, valG;
    FILE *in = fopen(nume_imagine, "rb");
    if(out == NULL)
    {
        printf("Eroare la deschiderea fisierului in care scriu\n");
        return;
    }
    if(in == NULL)
    {
        printf("Eroare la deschiderea imaginii pe care o testez\n");
        return;
    }
    fseek(in, 18, SEEK_SET);
    fread(&latime_img, 1, sizeof(unsigned int), in);
    fread(&inaltime_img, 1, sizeof(unsigned int), in);
    struct pixel *lin = (struct pixel *)calloc(inaltime_img * latime_img, sizeof(struct pixel));

    if(lin == NULL)
    {
        printf("Eroare la alocarea memoriei\n");
        return;
    }

    lin = incarca_imagine(nume_imagine);

    double red, green, blue, frecventa_estimata;
    red = 0;
    green = 0;
    blue = 0;
    frecventa_estimata = (inaltime_img * latime_img) / 256;
    int **f = (int **)calloc(256, sizeof(int *));

    if(f == NULL)
    {
        printf("Eroare la alocarea memoriei!\n");
        return;
    }

    for(i = 0;i < 256;i++)
    {
        f[i] = (int*) calloc(3, sizeof(int));
        if(f[i] == NULL)
        {
            printf("Eroare la alocarea memoriei");
            return;
        }
    }

    for(i = 0;i < inaltime_img;i++)
    {
        for(j = 0; j < latime_img;j++)
        {
            f[lin[i * latime_img + j].r][0] += 1;
            f[lin[i * latime_img + j].g][1] += 1;
            f[lin[i * latime_img + j].b][2] += 1;
        }
    }
    for(i = 0 ;i < 256;i++)
    {
        red += (f[i][0] - frecventa_estimata) *  (f[i][0] - frecventa_estimata)/ frecventa_estimata;
        green += pow(f[i][1] - frecventa_estimata, 2) / frecventa_estimata;
        blue += pow(f[i][2] - frecventa_estimata, 2) / frecventa_estimata;
    }

    fprintf(out, "Rezultatul testului chi-patrat pentru imaginea %s este: %f %f %f\n",nume_imagine, red, green, blue);

    for(i = 0;i < 256;i++)
        free(f[i]);
    free(f);
    fclose(in);
    free(lin);

}

struct pixel **Creaza_Matrice(FILE *img, int *inaltime, int *latime)
{
    int i, j;
    fseek(img, 18, SEEK_SET);
    fread(latime, sizeof(int), 1, img);
    fread(inaltime, sizeof(int), 1, img);
    struct pixel **MAP = (struct pixel **)calloc(*inaltime, sizeof(struct pixel *));

    if(MAP == NULL)
    {
        printf("Eroare la alocarea memoriei I\n");
        return NULL;
    }

    for(i = 0;i < *inaltime;i++)
    {
        MAP[i] = (struct pixel *)calloc(*latime, sizeof(struct pixel));
        if(MAP[i] == NULL)
        {
            printf("Eroare la alocarea memoriei");
            return NULL;
        }
    }

    int padding;
    if((*latime) % 4 != 0)
        padding = 4 - (3 * (*latime)) % 4;
    else padding = 0;

    fseek(img, 54, SEEK_SET);
    for(i = *inaltime - 1; i >= 0;i--)
    {
        for(j = 0;j < *latime;j++)
        {
            unsigned char aux[3];
            fread(aux, sizeof(unsigned char), 3, img);
            MAP[i][j].b = aux[0];
            MAP[i][j].g = aux[1];
            MAP[i][j].r = aux[2];
        }
        fseek(img, padding, SEEK_CUR);
    }
    return MAP;
}


int comparator(const void *a, const void *b)
{
    double va = ((struct detectie *) a)->cor;
    double vb = ((struct detectie *) b)->cor;
    if(va < vb)
        return 1;
    else if(va > vb)
        return -1;
    return 0;
}


int main()
{
    FILE *in = fopen("date_de_intrare.in", "rt");
    char nume_imagine[25], nume_sablon[10][25];
    int i;

    if(in == NULL)
    {
        printf("Nu am fisierul din care citesc datele de intrare!\n");
        return -1;
    }

    fscanf(in, "%s", nume_imagine);
    grayscale_image(nume_imagine, "img_grayscale.bmp"); //img_grayscale.bmp devine imaginea test grayscale

    for(i = 0;i < 10;i++)
    {
        fscanf(in,"%s", nume_sablon[i]);

    }
    fclose(in);

    char c;

    int j, inaltime, latime;     ///INCARCA IMAGINE
    FILE *img = fopen(nume_imagine,"rb");
    FILE *imgOUT = fopen("rezultat.bmp", "wb");
    FILE *imgGray = fopen("img_grayscale.bmp", "rb"); //deschid imaginea grayscale cu care lucrez

    if(img == NULL || imgOUT == NULL || imgGray == NULL)
    {
        printf("Eroare la deschiderea imaginilor");
        return -1;
    }
    while(fread(&c, sizeof(char), 1, img))
    {
        fwrite(&c, sizeof(char), 1, imgOUT);
        fflush(imgOUT);
    }

    struct pixel *cul = Creaza_culori();
    struct pixel **MAP = Creaza_Matrice(img, &inaltime, &latime);
    struct pixel **MAP_gray = Creaza_Matrice(imgGray, &inaltime, &latime);
    int padding;
    if(latime % 4 != 0)
        padding = 4 - (3 * latime) % 4;
    else padding = 0;

    int k = 0, nr_det = 0;
    struct detectie *D = NULL;
    int d_max = 0;
    struct detectie *det = NULL;
    int q;

    for(q = 0; q < 10; q++)
    {
        nr_det = 0;
        printf("fac template-matching pentru cifra %d \n", q);
        det = template_matching(MAP_gray, inaltime, latime, &nr_det, q, nume_sablon[q], 0.5);
        d_max += nr_det;
        D = (struct detectie *) realloc(D, d_max * sizeof(struct detectie));

        if(D == NULL)
        {
            printf("Eroare la realocarea memoriei in main\n");
            return -1;
        }
        for(i = 0; i < nr_det;i++)
        {
            D[k] = det[i];
            k++;
        }
        free(det); //dezaloc det
    }
    printf("Am efectuat template-mathing-ul\n");
    qsort(D, d_max, sizeof(struct detectie), comparator);

    printf("am sortat vectorul D\n");
    elimin_non_maximele(&D, &d_max);

    printf("am eliminat non-maximele\n");

    for(i = 0;i < d_max;i++)
    {
        int col = D[i].culoare;
        coloreaza_detectie(&MAP, inaltime, latime, D[i], cul[col]);
    }
    printf("Am colorat detectiile ramase\n");
    fseek(imgOUT, 54, SEEK_SET); ///SALVARE IMAGINE
    for(i = inaltime - 1; i >= 0;i--)
    {
        for(j = 0;j < latime;j++)
        {
            fwrite(&MAP[i][j], sizeof(struct pixel), 1, imgOUT);
            fflush(imgOUT);
        }
        if(i == 0)
            break;//am terminat
        fseek(imgOUT, padding, SEEK_CUR);
    }
    for(i = 0;i < inaltime;i++)
    {
        free(MAP[i]);
        free(MAP_gray[i]);
    }
    free(MAP);
    free(cul);
    free(MAP_gray);
    free(D);
    fclose(img);
    fclose(imgOUT);
    fclose(imgGray);

    return 0;
}
