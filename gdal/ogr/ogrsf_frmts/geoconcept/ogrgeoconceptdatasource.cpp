/******************************************************************************
 * $Id: ogrgeoconceptdatasource.cpp
 *
 * Name:     ogrgeoconceptdatasource.h
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Implements OGRGeoconceptDataSource class.
 * Language: C++
 *
 ******************************************************************************
 * Copyright (c) 2007, Geoconcept and IGN
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "ogrgeoconceptlayer.h"
#include "ogrgeoconceptdatasource.h"
#include "cpl_conv.h"
#include "cpl_string.h"

CPL_CVSID("$Id: ogrgeoconceptdatasource.cpp 00000 2007-11-03 11:49:22Z drichard $");

/************************************************************************/
/*                         OGRGeoconceptDataSource()                    */
/************************************************************************/

OGRGeoconceptDataSource::OGRGeoconceptDataSource()

{
    _papoLayers = NULL;
    _nLayers = 0;

    _pszGCT = NULL;
    _pszName = NULL;
    _pszDirectory = NULL;
    _pszExt = NULL;
    _papszOptions = NULL;

    _bSingleNewFile = FALSE;
    _bUpdate = FALSE;
    _hGXT = NULL;
}

/************************************************************************/
/*                        ~OGRGeoconceptDataSource()                    */
/************************************************************************/

OGRGeoconceptDataSource::~OGRGeoconceptDataSource()

{
    if ( _pszGCT )
    {
      CPLFree( _pszGCT );
    }
    if ( _pszName )
    {
      CPLFree( _pszName );
    }
    if ( _pszDirectory )
    {
      CPLFree( _pszDirectory );
    }
    if ( _pszExt )
    {
      CPLFree( _pszExt );
    }

    if ( _papoLayers )
    {
      for( int i = 0; i < _nLayers; i++ )
        delete _papoLayers[i];

      CPLFree( _papoLayers );
    }

    if( _hGXT )
    {
      Close_GCIO(&_hGXT);
    }

    if ( _papszOptions )
    {
      CSLDestroy( _papszOptions );
    }
}

/************************************************************************/
/*                                Open()                                */
/*                                                                      */
/*      Open an existing file.                                          */
/************************************************************************/

int OGRGeoconceptDataSource::Open( const char* pszName, int bUpdate )

{
    VSIStatBuf  stat;

/* -------------------------------------------------------------------- */
/*      Is the given path a directory or a regular file?                */
/* -------------------------------------------------------------------- */
    if( CPLStat( pszName, &stat ) != 0
        || (!VSI_ISDIR(stat.st_mode) && !VSI_ISREG(stat.st_mode)) )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                 "%s is neither a file or directory, Geoconcept access failed.\n",
                 pszName );

        return FALSE;
    }

    if( VSI_ISDIR(stat.st_mode) )
    {
        CPLDebug( "GEOCONCEPT",
                  "%s is a directory, Geoconcept access is not yet supported.",
                  pszName );

        return FALSE;
    }

    if( VSI_ISREG(stat.st_mode) )
    {
        _bSingleNewFile= FALSE;
        _bUpdate= bUpdate;
        _pszName= CPLStrdup( pszName );
        if( !LoadFile( _bUpdate? "a+t":"rt" ) )
        {
            CPLDebug( "GEOCONCEPT",
                      "Failed to open Geoconcept %s."
                      " It may be corrupt.",
                      pszName );

            return FALSE;
        }

        return TRUE;
    }

    return _nLayers > 0;
}

/************************************************************************/
/*                              LoadFile()                              */
/************************************************************************/

int OGRGeoconceptDataSource::LoadFile( const char *pszMode )

{
    OGRGeoconceptLayer *poFile;

    if( !_pszExt )
    {
      _pszExt = (char *)CPLGetExtension(_pszName);
      if( !EQUAL(_pszExt,"gxt") && !EQUAL(_pszExt,"txt") )
      {
        _pszExt = NULL;
        return FALSE;
      }
    }
    if( EQUAL(_pszExt,"txt") )
      _pszExt = CPLStrdup("txt");
    else if( EQUAL(_pszExt,"gxt") )
      _pszExt = NULL;
    else
      _pszExt = NULL;
    CPLStrlwr( _pszExt );

    if( !_pszDirectory )
        _pszDirectory = CPLStrdup( CPLGetPath(_pszName) );

    if( (_hGXT= Open_GCIO(_pszName,_pszExt,pszMode,_pszGCT))==NULL )
    {
      return FALSE;
    }

    /* Collect layers : */
    GCExportFileMetadata* Meta= GetGCMeta_GCIO(_hGXT);
    if( Meta )
    {
      int nC, iC, nS, iS;

      if( (nC= CountMetaTypes_GCIO(Meta))>0 )
      {
        GCType* aClass;
        GCSubType* aSubclass;

        for( iC= 0; iC<nC; iC++ )
        {
          if( (aClass= GetMetaType_GCIO(Meta,iC)) )
          {
            if( (nS= CountTypeSubtypes_GCIO(aClass)) )
            {
              for( iS= 0; iS<nS; iS++ )
              {
                if( (aSubclass= GetTypeSubtype_GCIO(aClass,iS)) )
                {
                  poFile = new OGRGeoconceptLayer;
                  if( poFile->Open(aSubclass) != OGRERR_NONE )
                  {
                    delete poFile;
                    return FALSE;
                  }

                  /* Add layer to data source layers list */
                  _papoLayers = (OGRGeoconceptLayer **)
                      CPLRealloc( _papoLayers,  sizeof(OGRGeoconceptLayer *) * (_nLayers+1) );
                  _papoLayers[_nLayers++] = poFile;

                  CPLDebug("GEOCONCEPT",
                           "nLayers=%d - last=[%s]",
                           _nLayers, poFile->GetLayerDefn()->GetName());
                }
              }
            }
          }
        }
      }
    }

    return TRUE;
}

/************************************************************************/
/*                               Create()                               */
/*                                                                      */
/*      Create a new dataset.                                           */
/*                                                                      */
/* Options (-dsco) :                                                    */
/*   EXTENSION : gxt|txt                                                */
/*   CONFIG : path to GCT file                                          */
/************************************************************************/

int OGRGeoconceptDataSource::Create( const char *pszName, char** papszOptions )

{
    char *conf;

    if( strlen(CPLGetExtension(pszName)) == 0 )
    {
        VSIStatBuf  sStat;

        if( VSIStat( pszName, &sStat ) == 0 )
        {
            CPLError( CE_Failure, CPLE_OpenFailed,
                      "Attempt to create dataset named %s,\n"
                      "but that is an existing file or directory.",
                      pszName );
            return FALSE;
        }
    }

    if( _pszName ) CPLFree(_pszName);
    _pszName = CPLStrdup( pszName );
    _papszOptions = CSLDuplicate( papszOptions );

    if( (conf= (char *)CSLFetchNameValue(papszOptions,"CONFIG")) )
    {
      _pszGCT = CPLStrdup(conf);
    }

    _pszExt = (char *)CSLFetchNameValue(papszOptions,"EXTENSION");
    if( _pszExt == NULL )
    {
        _pszExt = (char *)CPLGetExtension(pszName);
    }

/* -------------------------------------------------------------------- */
/*      Create a new single file.                                       */
/*      OGRGeoconceptDriver::CreateLayer() will do the job.             */
/* -------------------------------------------------------------------- */
    _pszName = CPLStrdup( pszName );
    _pszDirectory = CPLStrdup( CPLGetPath(pszName) );
    _bSingleNewFile = TRUE;

    if( !LoadFile( "wt" ) )
    {
        CPLDebug( "GEOCONCEPT",
                  "Failed to create Geoconcept %s.",
                  pszName );

        return FALSE;
    }

    return TRUE;
}

/************************************************************************/
/*                            CreateLayer()                             */
/*                                                                      */
/* Options (-lco) :                                                     */
/*   FEATURETYPE : TYPE.SUBTYPE                                         */
/************************************************************************/

OGRLayer *OGRGeoconceptDataSource::CreateLayer( const char * pszLayerName,
                                                OGRSpatialReference *poSRS /* = NULL */,
                                                OGRwkbGeometryType eType /* = wkbUnknown */,
                                                char ** papszOptions /* = NULL */ )

{
    GCTypeKind gcioFeaType;
    GCDim gcioDim;
    OGRGeoconceptLayer *poFile= NULL;
    char *pszFeatureType, **ft;
    int iLayer;

    if( poSRS == NULL && !_bUpdate) {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "SRS is mandatory of creating a Geoconcept Layer.\n"
                );
        return NULL;
    }

    /*
     * pszLayerName Class.Subclass if -nln option used, otherwise file name
     */
    if( !(pszFeatureType = (char *)CSLFetchNameValue(papszOptions,"FEATURETYPE")) )
    {
      if( !pszLayerName || !strchr(pszLayerName,'.') )
      {
        char pszln[512];

        snprintf(pszln,511,"%s.%s", pszLayerName? pszLayerName:"ANONCLASS",
                                    pszLayerName? pszLayerName:"ANONSUBCLASS");
        pszln[511]= '\0';
        pszFeatureType= pszln;
      }
      else
        pszFeatureType= (char *)pszLayerName;
    }

    if( !(ft= CSLTokenizeString2(pszFeatureType,".",0)) ||
        CSLCount(ft)!=2 )
    {
      CSLDestroy(ft);
      CPLError( CE_Failure, CPLE_AppDefined,
                "Feature type name '%s' is incorrect."
                "Correct syntax is : Class.Subclass.",
                pszFeatureType );
      return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Figure out what type of layer we need.                          */
/* -------------------------------------------------------------------- */
    gcioDim= v2D_GCIO;
    if( eType == wkbUnknown )
        gcioFeaType = vUnknownItemType_GCIO;
    else if( eType == wkbPoint )
        gcioFeaType = vPoint_GCIO;
    else if( eType == wkbLineString )
        gcioFeaType = vLine_GCIO;
    else if( eType == wkbPolygon )
        gcioFeaType = vPoly_GCIO;
    else if( eType == wkbMultiPoint )
        gcioFeaType = vPoint_GCIO;
    else if( eType == wkbMultiLineString )
        gcioFeaType = vLine_GCIO;
    else if( eType == wkbMultiPolygon )
        gcioFeaType = vPoly_GCIO;
    else if( eType == wkbPoint25D )
    {
        gcioFeaType = vPoint_GCIO;
        gcioDim= v3DM_GCIO;
    }
    else if( eType == wkbLineString25D )
    {
        gcioFeaType = vLine_GCIO;
        gcioDim= v3DM_GCIO;
    }
    else if( eType == wkbPolygon25D )
    {
        gcioFeaType = vPoly_GCIO;
        gcioDim= v3DM_GCIO;
    }
    else if( eType == wkbMultiPoint25D )
    {
        gcioFeaType = vPoint_GCIO;
        gcioDim= v3DM_GCIO;
    }
    else if( eType == wkbMultiLineString25D )
    {
        gcioFeaType = vLine_GCIO;
        gcioDim= v3DM_GCIO;
    }
    else if( eType == wkbMultiPolygon25D )
    {
        gcioFeaType = vPoly_GCIO;
        gcioDim= v3DM_GCIO;
    }
    else
    {
        CSLDestroy(ft);
        CPLError( CE_Failure, CPLE_NotSupported,
                  "Geometry type of '%s' not supported in Geoconcept files.\n",
                  OGRGeometryTypeToName(eType) );
        return NULL;
    }

    /*
     * As long as we use the CONFIG, creating a layer implies the
     * layer name to exist in the CONFIG as "Class.Subclass".
     * Removing the CONFIG, implies on-the-fly-creation of layers...
     */
    if( _nLayers > 0 )
      for( iLayer= 0; iLayer<_nLayers; iLayer++)
      {
        poFile= (OGRGeoconceptLayer*)GetLayer(iLayer);
        if( EQUAL(poFile->GetLayerDefn()->GetName(),pszFeatureType) )
        {
          break;
        }
        poFile= NULL;
      }
    if( !poFile )
    {
      GCSubType* aSubclass= NULL;
      if( GetGCMeta_GCIO(_hGXT) )
      {
        if( GetGCNbObjects_GCIO(_hGXT)>0 )
        {
          CSLDestroy(ft);
          CPLError( CE_Failure, CPLE_NotSupported,
                    "Adding layer '%s' to an existing dataset"
                    "not supported in Geoconcept driver.",
                    pszLayerName );
          return NULL;
        }
      }
      else
      {
        GCExportFileMetadata* m;

        if( !(m= CreateHeader_GCIO()) )
        {
          CSLDestroy(ft);
          return NULL;
        }
        SetMetaExtent_GCIO(m, CreateExtent_GCIO(HUGE_VAL,HUGE_VAL,-HUGE_VAL,-HUGE_VAL));
        SetGCMeta_GCIO(_hGXT, m);
      }
      if( FindFeature_GCIO(_hGXT, pszFeatureType) )
      {
        CSLDestroy(ft);
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Layer '%s' already exists.",
                  pszFeatureType );
        return NULL;
      }
      if( !AddType_GCIO(_hGXT, ft[0], -1L) )
      {
        CSLDestroy(ft);
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to add layer '%s'.",
                  pszFeatureType );
        return NULL;
      }
      if( !(aSubclass= AddSubType_GCIO(_hGXT, ft[0], ft[1], -1L, gcioFeaType, gcioDim)) )
      {
        CSLDestroy(ft);
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to add layer '%s'.",
                  pszFeatureType );
        return NULL;
      }
      poFile = new OGRGeoconceptLayer;
      if( poFile->Open(aSubclass) != OGRERR_NONE )
      {
        CSLDestroy(ft);
        delete poFile;
        return NULL;
      }

      /* complete feature type with private fields : */
      AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kIdentifier_GCIO, -100, vIntFld_GCIO, NULL, NULL);
      AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kClass_GCIO, -101, vMemoFld_GCIO, NULL, NULL);
      AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kSubclass_GCIO, -102, vMemoFld_GCIO, NULL, NULL);
      AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kNbFields_GCIO, -103, vIntFld_GCIO, NULL, NULL);
      AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kX_GCIO, -104, vRealFld_GCIO, NULL, NULL);
      AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kY_GCIO, -105, vRealFld_GCIO, NULL, NULL);
      /* user's fields will be added with Layer->CreateField() method ... */
      switch( gcioFeaType )
      {
        case vPoint_GCIO :
          break;
        case vLine_GCIO  :
          AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kXP_GCIO, -106, vRealFld_GCIO, NULL, NULL);
          AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kYP_GCIO, -107, vRealFld_GCIO, NULL, NULL);
          AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kGraphics_GCIO, -108, vUnknownItemType_GCIO, NULL, NULL);
          break;
        default          :
          AddSubTypeField_GCIO(_hGXT, ft[0], ft[1], -1L, kGraphics_GCIO, -108, vUnknownItemType_GCIO, NULL, NULL);
          break;
      }
      SetSubTypeGCHandle_GCIO(aSubclass,_hGXT);
      CSLDestroy(ft);

      /* Add layer to data source layers list */
      _papoLayers = (OGRGeoconceptLayer **)
                      CPLRealloc( _papoLayers,  sizeof(OGRGeoconceptLayer *) * (_nLayers+1) );
      _papoLayers[_nLayers++] = poFile;

      CPLDebug("GEOCONCEPT",
               "nLayers=%d - last=[%s]",
               _nLayers, poFile->GetLayerDefn()->GetName());
    }

/* -------------------------------------------------------------------- */
/*      Assign the coordinate system (if provided)                      */
/* -------------------------------------------------------------------- */
    if( poSRS != NULL )
        poFile->SetSpatialRef( poSRS );

    return poFile;
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int OGRGeoconceptDataSource::TestCapability( const char * pszCap )

{
    if( EQUAL(pszCap,ODsCCreateLayer) )
        return TRUE;
    else
        return FALSE;
}

/************************************************************************/
/*                              GetLayer()                              */
/************************************************************************/

OGRLayer *OGRGeoconceptDataSource::GetLayer( int iLayer )

{
    OGRLayer *poFile;
    if( iLayer < 0 || iLayer >= GetLayerCount() )
        poFile= NULL;
    else
        poFile= _papoLayers[iLayer];
    return poFile;
}
