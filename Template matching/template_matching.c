
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define max(a, b) ((a) > (b)) ? (a) : (b)
#define min(a, b) ((a) < (b)) ? (a) : (b)

typedef struct pixel{
    unsigned char b, g, r;
};

typedef struct detectie{
    int i, j;
    int culoare;
    double cor;
};

extern void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
   FILE *fin, *fout;
   unsigned int dim_img, latime_img, inaltime_img;
   unsigned char pRGB[3], aux;


   fin = fopen(nume_fisier_sursa, "rb");
   if(fin == NULL)
   	{
   		printf("nu am gasit imaginea sursa din care citesc");
   		return;
   	}

   fout = fopen(nume_fisier_destinatie, "wb+");

   fseek(fin, 2, SEEK_SET);
   fread(&dim_img, sizeof(unsigned int), 1, fin);

   fseek(fin, 18, SEEK_SET);
   fread(&latime_img, sizeof(unsigned int), 1, fin);
   fread(&inaltime_img, sizeof(unsigned int), 1, fin);

   //copiaza octet cu octet imaginea initiala in cea noua
	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
	fclose(fin);

	//calculam padding-ul pentru o linie
	int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;


	fseek(fout, 54, SEEK_SET);
	int i,j;
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			//citesc culorile pixelului
			fread(pRGB, 3, 1, fout);
			//fac conversia in pixel gri
			aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
			pRGB[0] = pRGB[1] = pRGB[2] = aux;
        	fseek(fout, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fout);
        	fflush(fout);
		}
		fseek(fout,padding,SEEK_CUR);
	}
	fclose(fout);
}

void incarca_fereastra(struct pixel ***rez, struct pixel **img, unsigned int x, unsigned int y, int inaltime, unsigned int latime, unsigned int inaltime_img, unsigned int latime_img)
{
    int i, j;

    //printf("inaltime : %d\n latime : %d\n", inaltime/2, latime/2);
    for(i = 0; i < inaltime; i++)
    {
        for(j = 0;j < latime;j++)
        {
            int dx = x - inaltime / 2 + i;
            int dy = y - latime / 2 + j;
            //printf("%d %d ",x - inaltime / 2 + i, y - latime / 2 + j );
            if(dx >= 0 && dy >= 0 && dx <inaltime_img && dy < latime_img)
            {
                  (*rez)[i][j].b = img[x - inaltime / 2 + i][y - latime / 2 + j].b;
                  (*rez)[i][j].g = img[x - inaltime / 2 + i][y - latime / 2 + j].g;
                  (*rez)[i][j].r = img[x - inaltime / 2 + i][y - latime / 2 + j].r;
            }
            else
            {
                (*rez)[i][j].b = 0;
                (*rez)[i][j].g = 0;
                (*rez)[i][j].r = 0;
            }
        }
    }
}

double deviatie_standard(struct pixel **img, int inaltime, int latime, double media_Pixelilor)
{
    double rez = 0;
    int i, j;
    double n = inaltime * latime;
    for(i = 0;i < inaltime;i++)
        for(j = 0;j < latime;j++)
        {
            double termen = 0;
            termen = (img[i][j].r - media_Pixelilor);
            termen = termen * termen;
            termen = termen / (n - 1);
            rez += termen;
        }
    rez = sqrt(rez);
    return rez;
}

void coloreaza_detectie(struct pixel ***M, int inaltime_img, int latime_img, struct detectie fereastra, struct pixel cul)
{
    const int n = 15, m = 11;
    int i, j;

    for(i = -(n / 2);i < (n / 2);i++)
    {
        if(!(fereastra.i + i < 0 || fereastra.i + i >= inaltime_img) && fereastra.j + (m/2) < latime_img && fereastra.j - (m / 2) >= 0)
        {
            (*M)[fereastra.i + i][fereastra.j + (m / 2)] = cul;
            (*M)[fereastra.i + i][fereastra.j - (m / 2)] = cul;
        }
    }
    for(j = -(m / 2);j < (m / 2);j++)
    {
        {
            if(fereastra.j - j >= 0 && fereastra.j + j < latime_img && fereastra.i - (n / 2) >= 0 && fereastra.i + (n / 2) < inaltime_img)
            {
                (*M)[fereastra.i - (n / 2)][fereastra.j + j] = cul;
                (*M)[fereastra.i + (n / 2)][fereastra.j + j] = cul;
            }
        }

    }
}


double corr(struct pixel **sablon, struct pixel **fereasra, int inaltime_sab, int latime_sab)
{
    double rez = 0;
    double n = inaltime_sab * latime_sab;
    double S_medie = 0, F_medie = 0, F_sigma, S_sigma;
    int i, j;
    for(i = 0;i < inaltime_sab;i++)
        for(j = 0;j < latime_sab;j++)
            {
                S_medie += sablon[i][j].r;
                F_medie += fereasra[i][j].r;
            }
    S_medie = S_medie / n;
    F_medie = F_medie / n;
    S_sigma = deviatie_standard(sablon, inaltime_sab, latime_sab, S_medie);
    F_sigma = deviatie_standard(fereasra, inaltime_sab, latime_sab, F_medie);
    for(i = 0;i < inaltime_sab;i++)
    {
        for(j = 0;j < latime_sab;j++)
        {
            double t1 = 0, t2 = 0, tf = 1/ (S_sigma * F_sigma * n);
            t1 = sablon[i][j].r - S_medie;
            t2 = fereasra[i][j].r - F_medie;

            tf = tf * (t1 * t2);
            rez += tf;
        }
    }
    return rez;
}



extern struct detectie *template_matching(struct pixel **MAP, int inaltime_img,int latime_img, int *numar_detectii, int cul, char *nume_sablon, double prag)
{
    int i, j;
    double correlation;

    grayscale_image(nume_sablon, "grayscale.bmp");

    FILE *sab = fopen("grayscale.bmp", "rb");


    unsigned int inaltime_sab, latime_sab;

    if(sab == NULL)
    {
        printf("Eroare la deschiderea sablonului\n");
        return;
    }
    fseek(sab, 18, SEEK_SET);
    fread(&latime_sab, sizeof(unsigned int), 1, sab);
    fread(&inaltime_sab, sizeof(unsigned int), 1, sab);


    struct pixel **sablon = (struct pixel **)calloc(inaltime_sab, sizeof(struct pixel *));

    if(sablon == NULL)
    {
        printf("Eroare la alocarea memoriei pentru sablon I\n");
        return NULL;
    }

    for(i = 0; i < inaltime_sab;i++)
    {
        sablon[i] = (struct pixel *)calloc(latime_sab, sizeof(struct pixel));
        if(sablon [i] == NULL)
        {
            printf("Eroare la alocarea memoriei pentru sablon II\n");
            return NULL;
        }
    }

    int padding_sab;
    if(latime_sab % 4 != 0)
        padding_sab = 4 - (3 * latime_sab) % 4;
    else padding_sab = 0;

    fseek(sab, 54, SEEK_SET);
    for(i = inaltime_sab - 1;i >= 0;i--)
    {
        for(j = 0; j < latime_sab; j++)
        {
            fread(&sablon[i][j], sizeof(struct pixel), 1, sab);
        }
        fseek(sab, padding_sab, SEEK_CUR);
    }


    fclose(sab);

    struct pixel **geam = NULL;
    geam = (struct pixel **)calloc(inaltime_sab, sizeof(struct pixel *));
    if(geam == NULL)
    {
        printf("Eroare la alocarea memoriei pentru fereastra I\n");
        return NULL;
    }
    for(i = 0;i < inaltime_sab;i++)
    {
        geam[i] = (struct pixel *)calloc(latime_sab, sizeof(struct pixel *));
        if(geam[i] == NULL)
        {
            printf("Eroare la alocarea memoriei pentru fereastraII \n");
            return NULL;
        }
    }
    struct detectie *det = NULL;


    for(i = 0;i < inaltime_img;i++)
    {
        for(j = 0;j < latime_img;j++)
        {
            incarca_fereastra(&geam, MAP, i, j, inaltime_sab, latime_sab, inaltime_img, latime_img); // incarc fereasta F(i, j) in geam
            correlation = corr(sablon, geam, inaltime_sab, latime_sab);
            if(correlation >= prag)// && i - 7 >= 0 && i + 7 < inaltime_img && j - 5 >= 0 && j + 5 < latime_img)
            {
                (*numar_detectii)++;
                det = (struct detectie *)realloc(det, (*numar_detectii) * sizeof(struct detectie));
                if(det == NULL)
                {
                    printf("Eroare la realocarea memoriei!\n");
                    return;
                }
                det[*numar_detectii - 1].i = i;
                det[*numar_detectii - 1].j = j;
                det[*numar_detectii - 1].cor = correlation;
                det[*numar_detectii - 1].culoare = cul;

            }
        }
    }
    for(i = 0; i < inaltime_sab;i++)
    {
        free(geam[i]);
        free(sablon[i]);
    }
    free(sablon);

    free(geam);

    return det;
}

double suprapunere(struct detectie a, struct detectie b)
{
    int x1, x2, x3, x4, y1, y2, y3, y4;

    x1 = a.i - 7;
    y1 = a.j - 5;
    x2 = a.i + 7;
    y2 = a.j + 5;
    x3 = b.i - 7;
    y3 = b.j - 5;
    x4 = b.i + 7;
    y4 = b.j + 5;

    int x5 = max(x1, x3);
    int y5 = max(y1, y3);
    int x6 = min(x2, x4);
    int y6 = min(y2, y4);
    if(x5 > x6 || y5 > y6) //nu se suprapun
        return 0;
    float latime = x6 - x5 + 1;
    float inaltime = y6 - y5 + 1;
    double rez = 0;
    rez = (latime * inaltime) / (165.0 + 165.0 - latime * inaltime);
    return rez;
}

void elimin_non_maximele(struct detectie **D, int *d_max)
{
    int i, j ,k;
    for(i = 0;i < *d_max - 1;i++)
    {
        for(j = i + 1;j < *d_max;j++)
        {
            if(suprapunere((*D)[i], (*D)[j]) > 0.2)
            {
                for(k = j;k < *d_max - 1;k++)
                    (*D)[k] = (*D)[k + 1];
                (*d_max)--;
            }
        }
    }
}

struct pixel *Creaza_culori()
{
    struct pixel *culoare = (struct pixel *)calloc(10, sizeof(struct pixel));
    culoare[0].r = 255, culoare[0].g = 0, culoare[0].b = 0;
    culoare[1].r = 255, culoare[1].g = 255, culoare[1].b = 0;
    culoare[2].r = 0, culoare[2].g = 255, culoare[2].b = 0;
    culoare[3].r = 0, culoare[3].g = 255, culoare[3].b = 255;
    culoare[4].r = 255, culoare[4].g = 0, culoare[4].b = 255;
    culoare[5].r = 0, culoare[5].g = 0, culoare[5].b = 255;
    culoare[6].r = 192, culoare[6].g = 192, culoare[6].b = 192;
    culoare[7].r = 255, culoare[7].g = 140, culoare[7].b = 0;
    culoare[8].r = 128, culoare[8].g = 0, culoare[8].b = 128;
    culoare[9].r = 128, culoare[9].g = 0, culoare[9].b = 0;
    return culoare;
}
