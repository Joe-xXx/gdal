/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.28
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.gdal.gdal;

public class GCP {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected GCP(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(GCP obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      gdalJNI.delete_GCP(swigCPtr);
    }
    swigCPtr = 0;
  }

  protected static long getCPtrAndDisown(GCP obj) {
    if (obj != null) obj.swigCMemOwn= false;
    return getCPtr(obj);
  }

  public void setGCPX(double value) {
    gdalJNI.GCP_GCPX_set(swigCPtr, value);
  }

  public double getGCPX() {
    return gdalJNI.GCP_GCPX_get(swigCPtr);
  }

  public void setGCPY(double value) {
    gdalJNI.GCP_GCPY_set(swigCPtr, value);
  }

  public double getGCPY() {
    return gdalJNI.GCP_GCPY_get(swigCPtr);
  }

  public void setGCPZ(double value) {
    gdalJNI.GCP_GCPZ_set(swigCPtr, value);
  }

  public double getGCPZ() {
    return gdalJNI.GCP_GCPZ_get(swigCPtr);
  }

  public void setGCPPixel(double value) {
    gdalJNI.GCP_GCPPixel_set(swigCPtr, value);
  }

  public double getGCPPixel() {
    return gdalJNI.GCP_GCPPixel_get(swigCPtr);
  }

  public void setGCPLine(double value) {
    gdalJNI.GCP_GCPLine_set(swigCPtr, value);
  }

  public double getGCPLine() {
    return gdalJNI.GCP_GCPLine_get(swigCPtr);
  }

  public void setInfo(String value) {
    gdalJNI.GCP_Info_set(swigCPtr, value);
  }

  public String getInfo() {
    return gdalJNI.GCP_Info_get(swigCPtr);
  }

  public void setId(String value) {
    gdalJNI.GCP_Id_set(swigCPtr, value);
  }

  public String getId() {
    return gdalJNI.GCP_Id_get(swigCPtr);
  }

  public GCP(double x, double y, double z, double pixel, double line, String info, String id) {
    this(gdalJNI.new_GCP(x, y, z, pixel, line, info, id), true);
  }

}
