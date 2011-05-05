/*
 *  $Id: BlockFileIO_2.c 3177 2008-04-01 14:47:24Z karstenm $
 *
 *  This file contains routines for reading attributes from files and writing
 *  attributes to files.
 */

/*
 *  Copyright 1993, 1994 University of British Columbia
 *
 *  Permission to use, copy, modify, distribute, and sell this software and its
 *  documentation for any purpose is hereby granted without fee, provided that
 *  the above copyright notice appears in all copies and that both that
 *  copyright notice and this permission notice appear in supporting
 *  documentation. UBC makes no representations about the suitability of this
 *  software for any purpose. It is provided "as is" without express or
 *  implied warranty.
 *
 *  Author: Arthur Pope, UBC Laboratory for Computational Intelligence
 */

/*
**  File IO is modified to permit better access to fMRI time series
**
**  modified: G.Lohmann, 26.4.1999
*/

/* From the Vista library: */
#include "viaio/Vlib.h"
#include "viaio/file.h"
#include "viaio/mu.h"
#include "viaio/os.h"
#include "viaio/VImage.h"
#include "viaio/VList.h"
#include "viaio/headerinfo.h"

/* From the standard C library: */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Macro used in WriteFile, WriteAttrList, etc.: */
#define FailTest(put)	    if ((put) == EOF) goto Fail

#define STRLEN 256

extern VBoolean ReadHeader (FILE *);
extern VAttrList ReadAttrList (FILE *);
extern VPackOrder MachineByteOrder  (void);
extern void SwapBytes (size_t, size_t, char *);


/* Seek start of binary data, i.e. skip header */
long
VGetHeaderLength(FILE *fp)
{
  size_t offset;
  int i;

  offset=0;
  fseek(fp,offset,SEEK_SET);
  while ((i=fgetc(fp)) != EOF) {
    /*      fprintf(stderr,"%c",i); */
    if (i == 014) goto next;
    offset++;
  }
next:
  i=fgetc(fp);
  offset+=2;
  fseek(fp,0L,SEEK_SET);
  return offset;
}

void
VImageInfoIni(VImageInfo *imageInfo)
{
  imageInfo->nbands = 0;
  imageInfo->nrows = 0;
  imageInfo->ncolumns = 0;
  imageInfo->repn = VUnknownRepn;

  imageInfo->pixelSize = 0;
  imageInfo->data=0L;
  imageInfo->length=0L;

  imageInfo->ori_nbands=0;
  imageInfo->ori_nrows=0;
  imageInfo->ori_ncolumns=0;
  imageInfo->top_margin=0;
  imageInfo->left_margin=0;

  imageInfo->spPH = 0;
  imageInfo->spPG = 0;
  imageInfo->subjects=0;
  imageInfo->ntimesteps=0;
  imageInfo->df=0.0;
  imageInfo->norm_mean=0.0;
  imageInfo->norm_sig=0.0;
  imageInfo->repetition_time=0;

  imageInfo->patient=(VString) VMalloc(STRLEN);
  memset(imageInfo->patient,0,STRLEN);
  imageInfo->patient[0]='N';

  imageInfo->modality=(VString) VMalloc(STRLEN);
  memset(imageInfo->modality,0,STRLEN);
  imageInfo->modality[0]='N';

  imageInfo->angle=(VString) VMalloc(STRLEN);
  memset(imageInfo->angle,0,STRLEN);
  imageInfo->angle[0]='N';

  imageInfo->voxel=(VString) VMalloc(STRLEN);
  memset(imageInfo->voxel,0,STRLEN);
  imageInfo->voxel[0]='N';

  imageInfo->name=(VString) VMalloc(STRLEN);
  memset(imageInfo->name,0,STRLEN);
  imageInfo->name[0]='N';

  imageInfo->fixpoint=(VString) VMalloc(STRLEN);
  memset(imageInfo->fixpoint,0,STRLEN);
  imageInfo->fixpoint[0]='N';

  imageInfo->ca=(VString) VMalloc(STRLEN);
  memset(imageInfo->ca,0,STRLEN);
  imageInfo->ca[0]='N';

  imageInfo->cp=(VString) VMalloc(STRLEN);
  memset(imageInfo->cp,0,STRLEN);
  imageInfo->cp[0]='N';

  imageInfo->location=(VString) VMalloc(STRLEN);
  memset(imageInfo->location,0,STRLEN);
  imageInfo->location[0]='N';

  imageInfo->orientation=(VString) VMalloc(STRLEN);
  memset(imageInfo->orientation,0,STRLEN);
  imageInfo->orientation[0]='N';

  imageInfo->extent=(VString) VMalloc(STRLEN);
  memset(imageInfo->extent,0,STRLEN);
  imageInfo->extent[0]='N';

  imageInfo->talairach=(VString) VMalloc(STRLEN);
  memset(imageInfo->talairach,0,STRLEN);
  imageInfo->talairach[0]='N';

  imageInfo->MPIL_vista_0=(VString) VMalloc(STRLEN);
  memset(imageInfo->MPIL_vista_0,0,STRLEN);
  imageInfo->MPIL_vista_0[0]='N';

  imageInfo->offsetHdr=0L;
}


VBoolean
VGetImageInfo(FILE *fp,VAttrList list,int object_id,VImageInfo *imageInfo)
{
  VBoolean  data_found,length_found,found;
  VAttrListPosn posn, subposn;
  VBundle b;
  int nobj=0;
  VLong xdata=0;
  VLong x=0;
  VLong lx=0;
  VDouble lf=0;
  VString str;

  /* if attr list not there, read it from disk */
  if (list == NULL) { 
    fseek(fp,0L,SEEK_SET);
    if (! ReadHeader (fp)) return FALSE;
    if (! (list = ReadAttrList (fp))) return FALSE;
  }

  nobj=0;
  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    nobj++;


    if (nobj-1 != object_id) continue;
    if (!VGetAttrValue (& posn, NULL, VBundleRepn, & b))
      VWarning("could not read bundle");

    /* get image dimensions */
    if (VLookupAttr (b->list, "nbands", & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's nbands attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->nbands = x;
      VDeleteAttr (& subposn);
    }
    else {
      imageInfo->nbands = 1;
/*       VWarning(" attribute's nbands attribute not found"); */
    }

    if (VLookupAttr (b->list, VNRowsAttr, & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's nrows attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->nrows = x;
      VDeleteAttr (& subposn);
    }
    else
      VWarning(" attribute's nrows attribute not found");



    if (VLookupAttr (b->list, VNColumnsAttr, & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's ncolumns attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->ncolumns = x;
      VDeleteAttr (& subposn);
    }
    else
      VWarning(" attribute's ncolumns attribute not found");



    /* get compression info */
    imageInfo->ori_nbands = 0;
    if (VLookupAttr (b->list, "ori_nbands", & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's ori_nbands attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->ori_nbands = x;
      VDeleteAttr (& subposn);
    }

    imageInfo->ori_nrows = 0;
    if (VLookupAttr (b->list, "ori_nrows", & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's ori_nrows attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->ori_nrows = x;
      VDeleteAttr (& subposn);
    }

    imageInfo->ori_ncolumns = 0;
    if (VLookupAttr (b->list, "ori_ncolumns", & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's ori_ncolumns attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->ori_ncolumns = x;
      VDeleteAttr (& subposn);
    }

    imageInfo->left_margin = 0;
    if (VLookupAttr (b->list, "left_margin", & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's left_margin attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->left_margin = x;
      VDeleteAttr (& subposn);
    }

    imageInfo->top_margin = 0;
    if (VLookupAttr (b->list, "top_margin", & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's top_margin attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->top_margin = x;
      VDeleteAttr (& subposn);
    }

    /* Extract any data and length attributes in the object's value: */
    if (data_found = VLookupAttr (b->list, VDataAttr, & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &xdata)) {
	VWarning ("VReadFile: "
		  "%s attribute's data attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->data = (size_t)xdata;
      VDeleteAttr (& subposn);
    }
    else
      VWarning(" attribute's data attribute not found");
    

    if (length_found = VLookupAttr (b->list, VLengthAttr, & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VLongRepn, &x)) {
	VWarning ("VReadFile: "
		  "%s attribute's length attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      imageInfo->length = x;
      VDeleteAttr (& subposn);
    }

    /* None or both must be present: */
    if (data_found ^ length_found) {
      VWarning ("VReadFile: %s attribute has %s but not %s",
		VGetAttrName (& posn),
		data_found ? "data" : "length",
		data_found ? "length" : "data");
      return FALSE;
    }


    /* get pixel repn */
    if (found = VLookupAttr (b->list,VRepnAttr, & subposn)) {
      if (! VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	VWarning ("VReadFile: "
		  "%s attribute's repn attribute incorrect",
		  VGetAttrName (& posn));
	return FALSE;
      }
      if (strncmp(str,"bit",3) == 0) {
	imageInfo->repn = VBitRepn;
	imageInfo->pixelSize = sizeof(VBit);
      }
      else if  (strncmp(str,"ubyte",5) == 0) {
	imageInfo->repn = VUByteRepn;
	imageInfo->pixelSize = sizeof(VUByte);
      }
      else if  (strncmp(str,"sbyte",5) == 0) {
	imageInfo->repn = VSByteRepn;
	imageInfo->pixelSize = sizeof(VSByte);
      }
      else if  (strncmp(str,"short",5) == 0) {
	imageInfo->repn = VShortRepn;
	imageInfo->pixelSize = sizeof(VShort);
      }
      else if  (strncmp(str,"long",4) == 0) {
	imageInfo->repn = VLongRepn;
	imageInfo->pixelSize = sizeof(VLong);
      }
      else if  (strncmp(str,"float",5) == 0) {
	imageInfo->repn = VFloatRepn;
	imageInfo->pixelSize = sizeof(VFloat);
      }
      else if  (strncmp(str,"double",6) == 0) {
	imageInfo->repn = VDoubleRepn;
	imageInfo->pixelSize = sizeof(VDouble);
      }
      else
	return FALSE;



      /* get fmri specifics */
      if (found = VLookupAttr (b->list,"patient", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->patient,0,STRLEN);
	  strncpy(imageInfo->patient,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"modality", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->modality,0,STRLEN);
	  strncpy(imageInfo->modality,str,strlen(str));
	}
      }
      if (found = VLookupAttr (b->list,"angle", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->angle,0,STRLEN);
	  strncpy(imageInfo->angle,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"voxel", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->voxel,0,STRLEN);
	  strncpy(imageInfo->voxel,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"name", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->name,0,STRLEN);
	  strncpy(imageInfo->name,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"fixpoint", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->fixpoint,0,STRLEN);
	  strncpy(imageInfo->fixpoint,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"ca", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->ca,0,STRLEN);
	  strncpy(imageInfo->ca,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"cp", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->cp,0,STRLEN);
	  strncpy(imageInfo->cp,str,strlen(str));
	}
      }
      if (found = VLookupAttr (b->list,"location", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->location,0,STRLEN);
	  strncpy(imageInfo->location,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"orientation", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->orientation,0,STRLEN);
	  strncpy(imageInfo->orientation,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"talairach", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->talairach,0,STRLEN);
	  strncpy(imageInfo->talairach,str,strlen(str));
	}
      }
      if (found = VLookupAttr (b->list,"MPIL_vista_0", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->MPIL_vista_0,0,STRLEN);
	  strncpy(imageInfo->MPIL_vista_0,str,strlen(str));
	  sscanf(str," repetition_time=%ld ",&lx);
	  imageInfo->repetition_time = lx;
	}
      }
      if (found = VLookupAttr (b->list,"extent", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VStringRepn, &str)) {
	  memset(imageInfo->extent,0,STRLEN);
	  strncpy(imageInfo->extent,str,strlen(str));
	}
      }

      if (found = VLookupAttr (b->list,"spPH", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VLongRepn, &lx)) {
	  imageInfo->spPH = lx;
	}
      }
      if (found = VLookupAttr (b->list,"spPG", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VLongRepn, &lx)) {
	  imageInfo->spPG = lx;
	}
      }
      if (found = VLookupAttr (b->list,"subjects", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VLongRepn, &lx)) {
	  imageInfo->subjects = lx;
	}
      }
      if (found = VLookupAttr (b->list,"ntimesteps", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VLongRepn, &lx)) {
	  imageInfo->ntimesteps = lx;
	}
      }
      if (found = VLookupAttr (b->list,"df", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VDoubleRepn, &lf)) {
	  imageInfo->df = lf;
	}
      }
      if (found = VLookupAttr (b->list,"norm_mean", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VDoubleRepn, &lf)) {
	  imageInfo->norm_mean = lf;
	}
      }
      if (found = VLookupAttr (b->list,"norm_sig", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VDoubleRepn, &lf)) {
	  imageInfo->norm_sig = lf;
	}
      }            
      if (found = VLookupAttr (b->list,"repetition_time", & subposn)) {
	if (VGetAttrValue (& subposn, NULL, VLongRepn, &lx)) {
	  imageInfo->repetition_time = lx;
	}
      }

    }
  }
  x = VGetHeaderLength(fp);
  imageInfo->offsetHdr = x;

  /* fprintf(stderr," info: %d %d %d\n",imageInfo->nbands,imageInfo->nrows,imageInfo->ncolumns); */

  return TRUE;
}


/*
** read several rows of data from a file using file pointer of type FILE
*/
VBoolean 
VReadBlockData (FILE *fp,VImageInfo *imageInfo,
		int row,int num_rows,VImage *buf)
{
  int i,column;
  size_t offset,offset2,ncolumns,nrows,nbands;
  size_t size,nitems;
  VPointer *dest_pp;


  /* Seek start of binary data */
  if (fseek(fp,imageInfo->offsetHdr,SEEK_SET) != 0) return FALSE;

  size     = imageInfo->pixelSize;
  ncolumns = imageInfo->ncolumns;
  nrows    = imageInfo->nrows;
  nbands   = imageInfo->nbands;

  /*
  fprintf(stderr," size; %d, ncol: %d  nrows: %d, nb: %d, fp: %x  data: %d \n",
  size,ncolumns,num_rows,nbands,fp,imageInfo->data);
  */

  nitems = ncolumns * num_rows;
  column = 0;

  offset  = imageInfo->data + (row*ncolumns + column) * size;
  offset2 = (nrows * ncolumns - num_rows * ncolumns) * size;

  fprintf(stderr, "offset: %d\n", offset);
  fprintf(stderr, "offset2: %d\n", offset2);

  /* read data from file */
  for (i=0; i<nbands; i++) {
    if (fseek(fp,offset,SEEK_CUR) != 0) return FALSE;
    dest_pp = (VPointer*)VPixelPtr((*buf),i,0,0);
    if (fread(dest_pp,size,nitems,fp) <= 0) return FALSE;
    dest_pp = (VPointer*)VPixelPtr((*buf),i,0,0);
    offset = offset2;
  }
  dest_pp = (VPointer*)VPixelPtr((*buf),0,0,0); 

  /* if little-endian, then swap bytes: */
  if (MachineByteOrder() == VLsbFirst) {
    for (i=0; i<nbands; i++) {
      dest_pp = (VPointer*)VPixelPtr((*buf),i,0,0);
      SwapBytes(nitems,size,(char*)dest_pp);
    }
  }
  return TRUE;
}



/*
** read several rows of data from a file using file pointer of type FILE
*/
VBoolean 
VReadBlockDataFD (int fd,VImageInfo *imageInfo,
		  int row,int num_rows,VImage *buf)
{
  int i,column;
  size_t offset,offset2,ncolumns,nrows,nbands;
  size_t size,nitems,nbytes;
  VPointer *dest_pp;

  /* Seek start of binary data */
  if (lseek(fd,imageInfo->offsetHdr,SEEK_SET) == -1) return FALSE;

  size     = imageInfo->pixelSize;
  ncolumns = imageInfo->ncolumns;
  nrows    = imageInfo->nrows;
  nbands   = imageInfo->nbands;

  /*
  fprintf(stderr," size; %d, ncol: %d  nrows: %d, nb: %d, fp: %x  data: %d \n",
  size,ncolumns,nrows,nbands,fp,imageInfo->data);
  */

  nitems = ncolumns * num_rows;
  nbytes = nitems * size;
  column = 0;

  offset  = imageInfo->data + (row*ncolumns + column) * size;
  offset2 = (nrows * ncolumns - num_rows * ncolumns) * size;

  /* read data from file */
  for (i=0; i<nbands; i++) {
    if (lseek(fd,offset,SEEK_CUR) == -1) return FALSE;

    dest_pp = (VPointer*)VPixelPtr((*buf),i,0,0);
    if (read(fd,dest_pp,nbytes) < 0) return FALSE;
    offset = offset2;
  }

  /* if little-endian, then swap bytes: */
  if (MachineByteOrder() == VLsbFirst) {
    for (i=0; i<nbands; i++) {
      dest_pp = (VPointer*)VPixelPtr((*buf),i,0,0);
      SwapBytes(nitems,size,(char*)dest_pp);
    }
  }

  return TRUE;
}



/*
** read several rows of data from a file in compressed format
*/
#ifdef MM
VBoolean 
VReadBlockDataCompress (FILE *fp,VImageInfo *imageInfo,
			int row,int num_rows,VImage *buf)
{
  int   i,r;
  int   ori_ncols;
  int   top_margin,left_margin;
  size_t data,offset,ncolumns,nrows,nbands;
  size_t size,nitems;
  VPointer *dest_pp;

  data     = imageInfo->data;
  size     = imageInfo->pixelSize;
  ncolumns = imageInfo->ncolumns;
  nrows    = imageInfo->nrows;
  nbands   = imageInfo->nbands;

  ori_ncols   = imageInfo->ori_ncolumns;
  top_margin  = imageInfo->top_margin;
  left_margin = imageInfo->left_margin;

  
  fprintf(stderr," size; %d, ncol: %d  nrows: %d, nb: %d, fp: %x  data: %d \n",
  size,ncolumns,nrows,nbands,fp,imageInfo->data);
  

  /* initialize buffer */
  dest_pp = VPixelPtr((*buf),0,0,0);
  memset(dest_pp,0,(int)(ori_ncols * num_rows * size));

  if (row+num_rows < top_margin) return TRUE;
  

  /* Seek start of binary data */
  if (fseek(fp,imageInfo->offsetHdr,SEEK_SET) == -1) return FALSE;

  nitems = ncolumns;

  /* read data from file */
  for (i=0; i<nbands; i++) {
    if (i%50 == 0) fprintf(stderr,"b: %5d\r",i);
    for (r=0; r<num_rows; r++) {
      if (r+row < top_margin) continue;

      dest_pp = VPixelPtr((*buf),i,r,left_margin);
      offset = data + ((row+r-top_margin)*ncolumns) * size;

      fprintf(stderr,"off: %5d, d: %6d,  %d %d %d %d\n\n",
	      offset,data,row,r,top_margin,left_margin);

      if (fseek(fp,offset,SEEK_SET) != 0) return FALSE;

      if (fread(dest_pp,size,nitems,fp) <= 0) return FALSE;
      dest_pp = VPixelPtr((*buf),i,r,left_margin);
      fprintf(stderr,"x: %d\n",(int)(*dest_pp));
      exit(0);
    }
  }

  return TRUE;
}
#endif

/*
** read several rows of data from a file
*/
void
VReadObjectData (FILE *fp,int object_id,VImage *image)
{
  static VImageInfo imageInfo;
  static VAttrList list=NULL;

  fseek(fp,0L,SEEK_SET);
  if (! ReadHeader (fp)) VError("error reading header"); 


  if (! (list = ReadAttrList (fp)))  
    VError("error reading attr list"); 
  
  if (! VGetImageInfo(fp,list,object_id,&imageInfo))
    VError(" error reading image info");

  if (imageInfo.nbands != VImageNBands((*image)))
    VError("incorrect number of bands");
  if (imageInfo.nrows != VImageNRows((*image))) 
    VError("incorrect number of rows");
  if (imageInfo.ncolumns != VImageNColumns((*image)))
    VError("incorrect number of columns");
  if (imageInfo.repn != VPixelRepn((*image))) 
    VError("incorrect pixel representation");
  
  if (! VReadBlockData (fp,&imageInfo,0,imageInfo.nrows,image))
    VError(" error reading data");
}


/*
** read several bands of data from a file
*/
VBoolean 
VReadBandData (FILE *fp,VImageInfo *imageInfo,
		int band,int num_bands,VImage *buf)
{
  int band1;
  size_t offset,ncolumns,nrows,nbands;
  size_t size,nitems;
  VPointer *dest_pp;

  size     = imageInfo->pixelSize;
  ncolumns = imageInfo->ncolumns;
  nrows    = imageInfo->nrows;
  nbands   = imageInfo->nbands;
  
  /*
  fprintf(stderr," size; %d, ncol: %d  nrows: %d, nb: %d, band: %d  data: %d \n",
  size,ncolumns,nrows,nbands,band,imageInfo->data);
  */

  /* Seek start of binary data */
  fseek(fp,0L,SEEK_SET);
  if (fseek(fp,(long)imageInfo->offsetHdr,SEEK_SET) != 0) return FALSE;

  band1 = num_bands + band;
  if (band1 > nbands) {
    VWarning(" illegal band addr: %d",band1);
    band1 = nbands;
    num_bands = band1 - band;
  }

  /* read all rows and all columns */

  nitems = ncolumns * nrows * num_bands;

  offset = imageInfo->data + (nrows * ncolumns * band) * size;
  if (fseek(fp,offset,SEEK_CUR) != 0) return FALSE;

  dest_pp = (VPointer*)VPixelPtr((*buf),0,0,0);
  if (fread(dest_pp,size,nitems,fp) <= 0) return FALSE;

  /* if little-endian, then swap bytes: */
  if (MachineByteOrder() == VLsbFirst) {
    dest_pp = (VPointer*)VPixelPtr((*buf),0,0,0);
    SwapBytes(nitems,size,(char*)dest_pp);
  }

  return TRUE;
}


/*
** read several bands of data from a file
*/
VBoolean 
VReadBandDataFD (int fd,VImageInfo *imageInfo,
		 int band,int num_bands,VImage *buf)
{
  size_t band1;
  size_t offset;
  size_t ncolumns,nrows,nbands;
  size_t size,nitems,nbytes;
  VPointer *dest_pp;

  size     = imageInfo->pixelSize;
  ncolumns = imageInfo->ncolumns;
  nrows    = imageInfo->nrows;
  nbands   = imageInfo->nbands;


  /* Seek start of binary data */
  lseek(fd,0L,SEEK_SET);
  if (lseek(fd,imageInfo->offsetHdr,SEEK_SET) == -1) {
    return FALSE;
  }

  band1 = num_bands + band;
  if (band1 > nbands) {
    VWarning(" illegal band addr: %d",band1);
    band1 = nbands;
    num_bands = band1 - band;
  }

  /* read all rows and all columns */

  nitems = ncolumns * nrows * num_bands;
  nbytes = nitems * size;

  offset = imageInfo->data + (nrows * ncolumns * (size_t)band) * size;

  if (lseek(fd,offset,SEEK_CUR) == -1) {
    return FALSE;
  }

  dest_pp = (VPointer*)VPixelPtr((*buf),0,0,0);
  if (read(fd,dest_pp,nbytes) < 0) {
    return FALSE;
  }

  /* if little-endian, then swap bytes: */
  if (MachineByteOrder() == VLsbFirst) {
    dest_pp = (VPointer*)VPixelPtr((*buf),0,0,0);
    SwapBytes(nitems,size,(char*)dest_pp);
  }

  return TRUE;
}

