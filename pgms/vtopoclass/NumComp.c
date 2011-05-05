/*
**  check if center voxel is a simple point,
**  return 1, if simple point,
**  return 0, otherwise
**
*/

#include <stdio.h>
#include <stdlib.h>

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

static int nad26[27] = {6,10,6,10,16,10,6,10,6,10,16,10,16,0,16,10,
		      16,10,6,10,6,10,16,10,6,10,6};

static int ad26[27][16] = {
  {1,3,4,9,10,12 },  /* 0 */
  {0,2,3,4,5,9,10,11,12 ,14}, /* 1 */
  {1,4,5,10,11 ,14}, /* 2 */
  {0,1,4,6,7,9,10,12 ,15,16}, /* 3 */
  {0,1,2,3,5,6,7,8,9,10,11,12 ,14,15,16,17}, /* 4 */
  {1,2,4,7,8,10,11 ,14,16,17}, /* 5 */
  {3,4,7,12 ,15,16}, /* 6 */
  {3,4,5,6,8,12 ,14,15,16,17}, /* 7 */
  {4,5,7 ,14,16,17}, /* 8 */
  {0,1,3,4,10,12 ,18,19,21,22}, /* 9 */
  {0,1,2,3,4,5,9,11,12 ,14,18,19,20,21,22,23}, /* 10 */
  {1,2,4,5,10 ,14,19,20,22,23}, /* 11 */
  {0,1,3,4,6,7,9,10 ,15,16,18,19,21,22,24,25}, /* 12 */
  {13},
  {1,2,4,5,7,8,10,11 ,16,17,19,20,22,23,25,26}, /* 14 */
  {3,4,6,7,12 ,16,21,22,24,25}, /* 15 */
  {3,4,5,6,7,8,12 ,14,15,17,21,22,23,24,25,26}, /* 16 */
  {4,5,7,8 ,14,16,22,23,25,26}, /* 17 */
  {9,10,12 ,19,21,22}, /* 18 */
  {9,10,11,12 ,14,18,20,21,22,23}, /* 19 */
  {10,11 ,14,19,22,23}, /* 20 */
  {9,10,12 ,15,16,18,19,22,24,25}, /* 21 */
  {9,10,11,12 ,14,15,16,17,18,19,20,21,23,24,25,26}, /* 22 */
  {10,11 ,14,16,17,19,20,22,25,26}, /* 23 */
  {12 ,15,16,21,22,25}, /* 24 */
  {12 ,14,15,16,17,21,22,23,24,26}, /* 25 */
  {14,16,17,22,23,25} /* 26 */
};


static int nad6[27] = {
  3,4,3,4,5,4,3,4,3,4,5,4,5,6,5,4,5,4,3,4,3,4,5,4,3,4,3
};

static int ad6[27][6] = {
  {1,3,9}, /* 0 */
  {0,2,4,10}, /* 1 */
  {1,5,11}, /* 2 */
  {0,4,6,12}, /* 3 */
  {1,3,5,7,13}, /* 4 */
  {2,4,8,14}, /* 5 */
  {3,7,15}, /* 6 */
  {4,6,8,16}, /* 7 */
  {5,7,17}, /* 8 */
  {0,10,12,18}, /* 9 */
  {1,9,11,13,19}, /* 10 */
  {2,10,14,20}, /* 11 */
  {3,9,13,15,21}, /* 12 */
  {4,10,12,14,16,22}, /* 13 */
  {5,11,13,17,23}, /* 14 */
  {6,12,16,24}, /* 15 */
  {7,13,15,17,25}, /* 16 */
  {8,14,16,26}, /* 17 */
  {9,19,21}, /* 18 */
  {10,18,20,22}, /* 19 */
  {11,19,23}, /* 20 */
  {12,18,22,24}, /* 21 */
  {13,19,21,23,25}, /* 22 */
  {14,20,22,26}, /* 23 */
  {15,21,25}, /* 24 */
  {16,22,24,26}, /* 25 */
  {17,23,25} /* 26 */
};



/*
** get number of 26-connected components in 3x3x3 neighbourhood
*/
int
VNumComp26(VBit src_pp[])
{
  int npixels;
  int i,j,k,l;
  int n;
  int label;
  int dest_pp[27];

  npixels = 27;
  for (i=0; i<npixels; i++) dest_pp[i] = 0;

  label = 0;
  for (i=0; i<npixels; i++) {

    if ((src_pp[i] == 0) || (dest_pp[i] > 0)) continue;

    label++;
    dest_pp[i] = label;

    n = 1;
    while (n > 0) {
      n = 0;

      for (j=0; j<npixels; j++) {

	if ((src_pp[j] == 0) || (dest_pp[j] > 0)) continue;

	for (k=0; k<nad26[j]; k++) {
	  l = ad26[j][k];
	  if (dest_pp[l] == label) {
	    dest_pp[j] = label;
	    n = 1;
	    goto next;
	  }
	}
	next: ;
      }
    }
  }

  return (int) label;
}


/*
** get number of 6-connected components in 3x3x3 neighbourhood
** 6-adjacent to the center voxel
*/
int
VANumComp6(VBit src_pp[])
{
  int npixels;
  int i,j,k,l;
  int n;
  int label;
  int dest_pp[27];
  int six[6] = {4,10,12,14,16,22}; /* 6-adjacent to center voxel */

  npixels = 27;
  for (i=0; i<npixels; i++) dest_pp[i] = 0;

  label = 0;
  for (i=0; i<6; i++) {

    if (src_pp[six[i]] == 0 || dest_pp[six[i]] > 0 ) continue;

    label++;
    dest_pp[six[i]] = label;

    n = 1;
    while (n > 0) {
      n = 0;

      for (j=i; j<npixels; j++) {

	if ((src_pp[j] == 0) || (dest_pp[j] > 0)) continue;

	for (k=0; k<nad6[j]; k++) {
	  l = ad6[j][k];
	  if (dest_pp[l] == label) {
	    dest_pp[j] = label;
	    n = 1;
	    goto next;
	  }
	}
	next: ;
      }
    }
  }

  return (int) label;
}



/*
** get number of 6-connected components in 3x3x3 neighbourhood
** 6-adjacent to the center voxel
*/
int
VBNumComp6(VBit src_pp[])
{
  int npixels;
  int i,j,k,l;
  int n;
  int label;
  int dest_pp[27];

  npixels = 27;
  for (i=0; i<npixels; i++) dest_pp[i] = 0;

  label = 0;
  for (i=0; i<27; i++) {

    if (src_pp[i] == 0 || dest_pp[i] > 0 ) continue;

    label++;
    dest_pp[i] = label;

    n = 1;
    while (n > 0) {
      n = 0;

      for (j=i; j<npixels; j++) {

	if ((src_pp[j] == 0) || (dest_pp[j] > 0)) continue;

	for (k=0; k<nad6[j]; k++) {
	  l = ad6[j][k];
	  if (dest_pp[l] == label) {
	    dest_pp[j] = label;
	    n = 1;
	    goto next;
	  }
	}
	next: ;
      }
    }
  }

  return (int) label;
}

