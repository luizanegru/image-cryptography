#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
struct pixel {
    unsigned char pB;
    unsigned char pG;
    unsigned char pR;
};

struct detectie {
    unsigned int lin;     //indicele i al liniei;
    unsigned int col;     //indicele j al coloane;
    double corr;          //corelatia dintre sablonul S si fereastra fI
    struct pixel culoare; //struct pixel culoare;
};

void liniarizare(char *numeFisier, struct pixel **arr_lin, unsigned int *latime_img, unsigned int *inaltime_img) {
    FILE *f = fopen(numeFisier, "rb");
    if (f == NULL) {
        printf("Eroare la deschiderea fisierului");
        return;
    }

    fseek(f, 18, SEEK_CUR);
    fread((latime_img), sizeof(unsigned int), 1, f);
    fread((inaltime_img), sizeof(unsigned int), 1, f);
    *arr_lin = (struct pixel *)malloc((*latime_img) * (*inaltime_img) * sizeof(struct pixel));

    unsigned int padding;
    if ((*latime_img) % 4 != 0)
        padding = 4 - (3 * (*latime_img)) % 4;
    else
        padding = 0;

    int i, j;
    unsigned char x;
    fseek(f, 54, SEEK_SET);
    for (i = (*inaltime_img) - 1; i > -1; i--) {
        for (j = 0; j < (*latime_img); j++) {
            fread(&x, sizeof(unsigned char), 1, f);
            (*arr_lin)[i * (*latime_img) + j].pB = x;
            fread(&x, sizeof(unsigned char), 1, f);
            (*arr_lin)[i * (*latime_img) + j].pG = x;
            fread(&x, sizeof(unsigned char), 1, f);
            (*arr_lin)[i * (*latime_img) + j].pR = x;
        }
        fseek(f, padding, SEEK_CUR);
    }
    fclose(f);
}

void xorShift32(unsigned int latime_img, unsigned int inaltime_img, unsigned int **arrXor, unsigned int *seed) {
    FILE *f = fopen("secret_key.txt", "r");
    unsigned int i, r, aux;

    if (f == NULL) {
        printf("Eroare la deschiderea fisierului");
        return;
    }
    fscanf(f, "%u", seed);

    unsigned int lungime = 0;

    lungime = 2 * latime_img * inaltime_img;
    *arrXor = (unsigned int *)malloc(lungime * sizeof(unsigned int));
    (*arrXor)[0] = (*seed); //Initializarea lui R0

    //Generarea numerelor intregi pe 32 de biti
    for (i = 1; i < lungime; i++) {
        (*arrXor)[i] = (*arrXor)[i - 1] ^ (*arrXor)[i - 1] << 13;
        (*arrXor)[i] = (*arrXor)[i] ^ (*arrXor)[i] >> 17;
        (*arrXor)[i] = (*arrXor)[i] ^ (*arrXor)[i] << 5;
    }
    fclose(f);
}
void permutare(unsigned int *arrXor, unsigned int latime_img, unsigned int inaltime_img, unsigned int seed, unsigned int **perm) {
    FILE *f = fopen("secret_key.txt", "r");
    unsigned int i, r, aux, k;

    if (f == NULL) {
        printf("Eroare la deschiderea fisierului");
        return;
    }
    fscanf(f, "%u", &seed);
    *perm = (unsigned int *)malloc(latime_img * inaltime_img * sizeof(unsigned int));

    for (i = 0; i < (latime_img) * (inaltime_img); i++) {
        (*perm)[i] = i;
    }

    xorShift32(latime_img, inaltime_img, &arrXor, &seed);
    for (i = (latime_img * inaltime_img) - 1; i >= 1; i--) {
        r = arrXor[(latime_img * inaltime_img) - i] % (i + 1);
        aux = (*perm)[r];
        (*perm)[r] = (*perm)[i];
        (*perm)[i] = aux;
    }
}
void permutarea_imaginii(struct pixel *arr_lin, struct pixel **arr_img_perm) {

    unsigned int inaltime_img, latime_img, i, seed, arrXor, *perm;
    liniarizare("imagine_initiala.bmp", &arr_lin, &latime_img, &inaltime_img);
    permutare(&arrXor, latime_img, inaltime_img, seed, &perm);
    *arr_img_perm = (struct pixel *)malloc(latime_img * inaltime_img * sizeof(struct pixel));

    for (i = 0; i < (latime_img * inaltime_img); i++) {
        (*arr_img_perm)[perm[i]] = arr_lin[i];
    }
}

void primire_chei(char *numeFisierChei, unsigned int *R0, unsigned int *SV) {
    FILE *f = fopen(numeFisierChei, "r");
    if (f == NULL) {
        printf("Eroare la deschiderea fisierului");
        return;
    }
    fscanf(f, "%u", R0);
    fscanf(f, "%u", SV);
    fclose(f);
}

void dimensiuni(char *numeFisierImg, unsigned int *latime_img, unsigned int *inaltime_img) {
    FILE *g = fopen(numeFisierImg, "rb");
    if (g == NULL) {
        printf("Eroare la deschiderea fisierului");
        return;
    }
    fseek(g, 18, SEEK_SET);
    fread(latime_img, sizeof(unsigned int), 1, g);
    fread(inaltime_img, sizeof(unsigned int), 1, g);

    fclose(g);
}

struct pixel pixelXORint(unsigned int x, struct pixel p1) {
    unsigned char X0, X1, X2;
    X0 = (unsigned char)x & 0xFF;
    x >>= 8;
    X1 = (unsigned char)x & 0xFF;
    x >>= 8;
    X2 = (unsigned char)x & 0xFF;

    struct pixel p2;
    (p2).pB = p1.pB ^ X0;

    (p2).pG = p1.pG ^ X1;

    (p2).pR = p1.pR ^ X2;
    return p2;
}

struct pixel pixelXORpixel(struct pixel p1, struct pixel p2) {
    struct pixel p3;
    (p3).pB = (p1.pB ^ p2.pB);
    (p3).pG = (p1.pG ^ p2.pG);
    (p3).pR = (p1.pR ^ p2.pR);
    return p3;
}

void criptare(struct pixel *arr_img_perm, unsigned int *arrXor, struct pixel **arr_img_crip, struct pixel *arr_lin) {
    unsigned int SV, R0, latime_img, inaltime_img, i = 0, k;
    // struct pixel arr_img_perm;

    primire_chei("secret_key.txt", &R0, &SV);
    dimensiuni("imagine_initiala.bmp", &latime_img, &inaltime_img);

    xorShift32(latime_img, inaltime_img, &arrXor, &R0);
    permutarea_imaginii(arr_lin, &arr_img_perm);

    *arr_img_crip = (struct pixel *)malloc((latime_img * inaltime_img) * sizeof(struct pixel));

    k = inaltime_img * latime_img;

    (*arr_img_crip)[0] = pixelXORint(SV, arr_img_perm[0]);
    (*arr_img_crip)[0] = pixelXORint(arrXor[k], (*arr_img_crip)[0]);
    for (i = 1; i < inaltime_img * latime_img; i++) {
        (*arr_img_crip)[i] = pixelXORpixel((*arr_img_crip)[i - 1], arr_img_perm[i]);

        (*arr_img_crip)[i] = pixelXORint(arrXor[k + i], (*arr_img_crip)[i]);
    }
}

///////  DECRIPTARE ///////
void permutare_inversa(unsigned int *perm, unsigned int **perm_inv) {
    unsigned int latime_img, inaltime_img, i, arrXor, seed;
    dimensiuni("imagine_criptata.bmp", &latime_img, &inaltime_img);
    permutare(&arrXor, latime_img, inaltime_img, seed, &perm);
    *perm_inv = (unsigned int *)malloc(latime_img * inaltime_img * sizeof(unsigned int));

    for (i = 0; i < inaltime_img * latime_img; i++) {
        (*perm_inv)[perm[i]] = i;
    }
}
void substitutia_inversa(struct pixel **arr_C_prim, struct pixel *arr_C) {

    unsigned int latime_img, inaltime_img, R0, SV, i, lungime, *arrXor, seed;
    primire_chei("secret_key.txt", &R0, &SV);
    liniarizare("imagine_criptata.bmp", &arr_C, &latime_img, &inaltime_img);
    lungime = latime_img * inaltime_img;
    xorShift32(latime_img, inaltime_img, &arrXor, &seed);

    *arr_C_prim = (struct pixel *)malloc(lungime * sizeof(struct pixel));
    (*arr_C_prim)[0] = pixelXORint(SV, arr_C[0]);
    (*arr_C_prim)[0] = pixelXORint(arrXor[lungime], (*arr_C_prim)[0]);

    for (i = 1; i < lungime; i++) {
        (*arr_C_prim)[i] = pixelXORpixel(arr_C[i - 1], arr_C[i]);
        (*arr_C_prim)[i] = pixelXORint(arrXor[lungime + i], (*arr_C_prim)[i]);
    }
}

void decriptare(struct pixel **arr_D, struct pixel *arr_C_prim) {
    unsigned int latime_img, inaltime_img, i, *perm, *perm_inv;
    struct pixel arr_C;
    dimensiuni("imagine_criptata.bmp", &latime_img, &inaltime_img);

    permutare_inversa(perm, &perm_inv);
    *arr_D = (struct pixel *)malloc(latime_img * inaltime_img * sizeof(struct pixel));
    substitutia_inversa(&arr_C_prim, &arr_C);

    for (i = 0; i < inaltime_img * latime_img; i++) {
        (*arr_D)[perm_inv[i]] = arr_C_prim[i];
    }
}

void afisare(char *numeFisier_sursa, char *numeFisier_destinatie, struct pixel *arr_img_crip, unsigned int latime_img,
             unsigned int inaltime_img) {
    FILE *fin = fopen(numeFisier_sursa, "rb");
    FILE *fout = fopen(numeFisier_destinatie, "wb");
    if (fin == NULL) {
        printf("Eroare la deschiderea fisierului");
        return;
    }
    if (fout == NULL) {
        printf("Eroare la deschiderea fisierului");
        return;
    }
    fseek(fin, 18, SEEK_SET);
    fread((&latime_img), sizeof(unsigned int), 1, fin);
    fread((&inaltime_img), sizeof(unsigned int), 1, fin);

    int i, j, k;
    unsigned char x, y = 0;
    unsigned int padding;
    if (latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;
    fseek(fin, 0, SEEK_SET);
    for (i = 0; i < 54; i++) {
        fread(&x, sizeof(unsigned char), 1, fin);
        fwrite(&x, sizeof(unsigned char), 1, fout);
    }

    for (i = inaltime_img - 1; i > -1; i--) {
        for (j = 0; j < latime_img; j++) {
            fwrite(&arr_img_crip[i * latime_img + j], sizeof(struct pixel), 1, fout);
        }
        for (k = 0; k < padding; k++) {
            fwrite(&y, sizeof(unsigned char), 1, fout);
        }
    }
    fclose(fin);
    fclose(fout);
}
void test(char *numeFisierImg, float *XR, float *XG, float *XB) {
    unsigned int i, j, inaltime_img, latime_img, *frB, *frG, *frR;
    float f_, sumB, sumG, sumR;

    frB = (unsigned int *)malloc(256 * sizeof(unsigned int));
    frG = (unsigned int *)malloc(256 * sizeof(unsigned int));
    frR = (unsigned int *)malloc(256 * sizeof(unsigned int));

    unsigned char x;
    FILE *f = fopen(numeFisierImg, "rb");
    if (f == NULL) {
        printf("Eroare la deschiderea fiaierului");
        return;
    }

    fseek(f, 18, SEEK_SET);
    fread((&latime_img), sizeof(unsigned int), 1, f);
    fread((&inaltime_img), sizeof(unsigned int), 1, f);
    f_ = (latime_img * inaltime_img) / 256;

    fseek(f, 54, SEEK_SET);
    for (i = 0; i < inaltime_img; i++) {
        for (j = 0; j < latime_img; j++) {
            fread(&x, sizeof(unsigned char), 1, f);
            {
                frB[x]++;
            }

            fread(&x, sizeof(unsigned char), 1, f);
            {
                frG[x]++;
            }
            fread(&x, sizeof(unsigned char), 1, f);
            {
                frR[x]++;
            }
        }
    }
    for (i = 0; i < 256; i++) {
        sumB = (frB[i] - f_) * (frB[i] - f_);
        sumB = sumB / f_;
        (*XB) = (*XB) + sumB;

        sumG = (frG[i] - f_) * (frG[i] - f_);
        sumG = sumG / f_;
        (*XG) = (*XG) + sumG;

        sumR = (frR[i] - f_) * (frR[i] - f_);
        sumR = sumR / f_;
        (*XR) = (*XR) + sumR;
    }

    fclose(f);
}

////// Template matching ///////
void colorare(struct pixel **arr_lin_color, unsigned int linie, unsigned int coloana, struct pixel *C, char *numeImg, char *numeSablon) {

    unsigned int latime_img, inaltime_img, inaltime_sab, latime_sab, i, j, fixare;
    dimensiuni(numeImg, &latime_img, &inaltime_img);
    dimensiuni(numeSablon, &latime_sab, &inaltime_sab);

    for (i = 0; i < latime_sab; i++) {
        //am colorat linia de sus a ferestrei;
        ((*arr_lin_color))[latime_img * linie + i + coloana].pB = (*C).pB;
        (*arr_lin_color)[latime_img * linie + i + coloana].pG = (*C).pG;
        (*arr_lin_color)[latime_img * linie + i + coloana].pR = (*C).pR;

        //coloram linia de jos a ferestrei
        (*arr_lin_color)[latime_img * linie + (inaltime_sab - 1) * latime_img + i + coloana].pB = (*C).pB;
        (*arr_lin_color)[latime_img * linie + (inaltime_sab - 1) * latime_img + i + coloana].pG = (*C).pG;
        (*arr_lin_color)[latime_img * linie + (inaltime_sab - 1) * latime_img + i + coloana].pR = (*C).pR;
    }

    for (i = 0; i < inaltime_sab - 1; i++) {

        //coloram coloana din stanga;
        (*arr_lin_color)[latime_img * (linie + 1 + i) + coloana].pB = (*C).pB;

        (*arr_lin_color)[latime_img * (linie + 1 + i) + coloana].pG = (*C).pG;

        (*arr_lin_color)[latime_img * (linie + 1 + i) + coloana].pR = (*C).pR;

        //coloram coloana din dreapta;
        (*arr_lin_color)[latime_img * (linie + 1 + i) + coloana + latime_sab - 1].pB = (*C).pB;

        (*arr_lin_color)[latime_img * (linie + 1 + i) + coloana + latime_sab - 1].pG = (*C).pG;
        (*arr_lin_color)[latime_img * (linie + 1 + i) + coloana + latime_sab - 1].pR = (*C).pR;
    }
}

// "n"  reprezinta numarul de pixeli din sablonul S = latime_sab * inaltime_sab
// "i" reprezinta linia, iar "j" reprezinta coloana din sablonul S
// "Sij" reprezinta valoarea intensitatii grayscale a pixelului de la linia i coloana j in sablonul S
//"S_" reprezinta media valorilor intansitatiilor grayscale a pixelilor din sablonul S
//"sigmaS" reprezinta deviatia standard a valorilor intensitatilor grayscale a pixelilor in sablonul S
//fIij reprezinta valoarea intensitatii grayscale a pixelului de pe linia i si coloana j din fereastra fI
//"fI_" reprezinta media valorilor intensitatilor grayscale a pixelilor din fereastra fI
//"sigmafI" reprezinta deviatia standard a valorilor intensitatilor grayscale a pixelilor din fereastra fI
double corelatie(char *numeImg, char *numeSablon, unsigned int linie, unsigned int coloana, struct pixel *arr_lin_gri, struct pixel *arr_lin_sab) {
    unsigned int latime_img, inaltime_img, latime_sab, inaltime_sab, n, i, j;

    FILE *f = fopen(numeImg, "rb");
    FILE *g = fopen(numeSablon, "rb");

    if (f == NULL || g == NULL) {
        printf("Eroare la deschiderea fisierului imagine/ sablon");
        return 1;
    }
    fseek(f, 18, SEEK_CUR);
    fread((&latime_img), sizeof(unsigned int), 1, f);
    fread((&inaltime_img), sizeof(unsigned int), 1, f);

    fseek(g, 18, SEEK_CUR);
    fread((&latime_sab), sizeof(unsigned int), 1, g);
    fread((&inaltime_sab), sizeof(unsigned int), 1, g);

    double fI_ = 0, S_ = 0, sigmafI = 0, sigmaS = 0, corr = 0;
    // numarul de pixeli
    n = inaltime_sab * latime_sab;

    // media valorilor intensitatilor grayscale a pixelilor din sablon

    for (i = 0; i < inaltime_sab; ++i)
        for (j = 0; j < latime_sab; ++j)
            S_ = S_ + arr_lin_sab[i * latime_sab + j].pR;
    S_ = S_ / n;

    // deviatia standard a valorilor intensitatilor grayscale a pixelilor din sablon

    for (i = 0; i < inaltime_sab; ++i)
        for (j = 0; j < latime_sab; ++j) {
            double derivatie = arr_lin_sab[i * latime_sab + j].pR - S_;
            derivatie = derivatie * derivatie;
            sigmaS = sigmaS + derivatie;
        }
    sigmaS = sigmaS / (n - 1);
    sigmaS = sqrt(sigmaS);

    // media valorilor intensitatilor grayscale a pixelilor din fereastra

    for (i = 0; i < inaltime_sab; ++i)
        for (j = 0; j < latime_sab; ++j)
            fI_ = fI_ + arr_lin_gri[(i + (linie)) * latime_img + (j + (coloana))].pR;
    fI_ = fI_ / n;

    // deviatia standard a valorilor intensitatilor grayscale a pixelilor din fereastra

    for (i = 0; i < inaltime_sab; ++i)
        for (j = 0; j < latime_sab; ++j) {
            double derivatie = arr_lin_gri[(i + (linie)) * latime_img + (j + (coloana))].pR - fI_;
            derivatie = derivatie * derivatie;
            sigmafI = sigmafI + derivatie;
        }
    sigmafI = sigmafI / (n - 1);
    sigmafI = sqrt(sigmafI);

    for (i = 0; i < inaltime_sab; ++i)
        for (j = 0; j < latime_sab; ++j) {
            unsigned char intensitateImg = arr_lin_gri[(i + (linie)) * latime_img + (j + (coloana))].pR;
            unsigned char intensitateSab = arr_lin_sab[i * latime_sab + j].pR;

            double aux = intensitateImg - fI_;
            aux = aux * (intensitateSab - S_);
            aux = aux / sigmafI;
            aux = aux / sigmaS;
            corr = corr + aux;
        }
    corr = corr / n;

    return corr;
}
void glisare(char *numeImgGri, char *numeSablonGri, struct detectie **D, unsigned int *n, double prag, struct pixel *C) {
    FILE *f = fopen(numeImgGri, "rb");
    FILE *g = fopen(numeSablonGri, "rb");

    if (f == NULL || g == NULL) {
        printf("Eroare la deschiderea fisierului img sau sablon ");
        return;
    }
    unsigned int latime_img, inaltime_img, latime_sab, inaltime_sab, i, j;
    double corr = 0;

    fseek(f, 18, SEEK_CUR);
    fread((&latime_img), sizeof(unsigned int), 1, f);
    fread((&inaltime_img), sizeof(unsigned int), 1, f);

    fseek(g, 18, SEEK_CUR);
    fread((&latime_sab), sizeof(unsigned int), 1, g);
    fread((&inaltime_sab), sizeof(unsigned int), 1, g);
    struct pixel *arr_lin_gri, *arr_lin_sab, *arr_lin;
    struct detectie *temp;
    *D = NULL;
    (*n) = 0;

    for (i = 0; i < inaltime_img - inaltime_sab; i++) {
        for (j = 0; j < latime_img - latime_sab; j++) {
            corr = corelatie(numeImgGri, numeSablonGri, i, j, arr_lin_gri, arr_lin_sab);

            if (corr > (prag)) {
                colorare(&arr_lin, i, j, C, numeImgGri, numeSablonGri);

                (*n)++;
                temp = (struct detectie *)realloc(*D, (*n) * sizeof(struct detectie));

                if (temp == NULL) {
                    printf("Nu s-a putut realoca");
                    free(*D);
                    return;
                }
                else {
                    *D = temp;
                    (*D)[(*n) - 1].lin = i;
                    (*D)[(*n) - 1].col = j;
                    (*D)[(*n) - 1].corr = corr;
                    (*D)[(*n) - 1].culoare.pR = (*C).pR;
                    (*D)[(*n) - 1].culoare.pG = (*C).pG;
                    (*D)[(*n) - 1].culoare.pB = (*C).pB;
                }
            }
        }
    }
    fclose(f);
    fclose(g);
}

int compare(const void *a, const void *b) {
    struct detectie va = *(struct detectie *)a;
    struct detectie vb = *(struct detectie *)b;
    if (va.corr > vb.corr)
        return 1;
    else
        return -1;
}

int main() {
    char numeImg[20], numeImgLiniarizata[20], numeImgCriptata[20], numeImgDecriptata[20];
    struct pixel *arr_img_crip, arr_img_perm;
    struct pixel arr_lin, *arr_liniarizat;
    unsigned int latime_img, inaltime_img, arrXor;
    struct pixel *arr_D, arr_C_prim;

    printf("Numele imaginii initiale este:");
    scanf("%s", numeImg);
    printf("\n");
    printf("Numele imaginii liniarizate este:");
    scanf("%s", numeImgLiniarizata);
    printf("\n");
    printf("Numele imaginii criptate este:");
    scanf("%s", numeImgCriptata);
    printf("\n");
    printf("Numele imaginii deccriptate este:");
    scanf("%s", numeImgDecriptata);

    liniarizare(numeImg, &arr_liniarizat, &latime_img, &inaltime_img);
    afisare(numeImg, numeImgLiniarizata, arr_liniarizat, latime_img, inaltime_img);

    criptare(&arr_img_perm, &arrXor, &arr_img_crip, &arr_lin);
    afisare(numeImg, numeImgCriptata, arr_img_crip, latime_img, inaltime_img);

    decriptare(&arr_D, &arr_C_prim);
    afisare(numeImgCriptata, numeImgDecriptata, arr_D, latime_img, inaltime_img);

    float XB_i = 0, XG_i = 0, XR_i = 0;

    test(numeImg, &XR_i, &XG_i, &XB_i);
    printf("Chi-squared test on RGB channels for: %s", numeImg);
    printf("\n");
    printf("R: %f  \nB: %f \nG: %f \n", XR_i, XG_i, XB_i);

    float XB_c = 0, XG_c = 0, XR_c = 0;

    test(numeImgCriptata, &XR_c, &XG_c, &XB_c);
    printf("Chi-squared test on RGB channels for: %s", numeImgCriptata);
    printf("\n");
    printf("R: %f  \nB: %f \nG: %f \n", XR_c, XG_c, XB_c);

    struct pixel *arr_lin_colorare, *arr_lin_sab, *arr_lin_gri;
    struct detectie fI;
    struct detectie *D;
    struct pixel C;
    unsigned int latime_sab, inaltime_sab, linie, coloana, n, i, j, latime_img_gri, inaltime_img_gri;
    double corr, prag = 0.5;
    unsigned int elem_arr_det;
    char numeImgGri[20];
    char numeImgTest[20];
    char numeSablon[20];
    printf("Numele imaginii este grayscale:");
    scanf("%s ", numeImgGri);

    printf("Numele imaginii test este:");
    scanf("%s ", numeImgTest);

    liniarizare(numeImgTest, &arr_lin_colorare, &latime_img, &inaltime_img);
    liniarizare(numeImgGri, &arr_lin_gri, &latime_img_gri, &inaltime_img_gri);

    for (i = 0; i <= 9; i++) {
        printf("Numele sablonului este:");
        scanf("%s ", numeSablon);
        printf("Culoarea pentru sablonul actual este");
        scanf("%c, %c, %c ", (C).pR, (C).pG, (C).pB);
        liniarizare(numeSablon, &arr_lin_sab, &latime_sab, &inaltime_sab);
        glisare(numeImgGri, numeImgGri, &D, &n, prag, &C);
    }

    afisare(numeImgTest, numeImgTest, arr_lin_colorare, latime_img, inaltime_img);

    qsort(D, n, sizeof(struct detectie), compare);

    return 0;
}
