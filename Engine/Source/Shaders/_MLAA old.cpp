/******************************************************************************/
#define MLAAThreshold  0.1f
#define MLAAThreshold2 (2.5f/255)

inline Flt ColDiff(Vec a, Vec b)
{
	return Dist2(a,b)>(MLAAThreshold*MLAAThreshold);
}

Vec4 EdgeSoftenEdgeDetect_PS(IF_IS_PIXEL
                             Vec2 inTex:TEXCOORD):COLOR
{
	Vec  col  =Tex(Col,inTex                  ).rgb,
	     col_x=Tex(Col,inTex+PixSize*Vec2(1,0)).rgb,
	     col_y=Tex(Col,inTex+PixSize*Vec2(0,1)).rgb;
	Vec2 edge =Vec2(ColDiff(col,col_x),ColDiff(col,col_y));
 //clip(edge.x+edge.y-1); // if both are zero then don't output stencil value
	return Vec4(edge,0,0);
}

Vec4 EdgeSoftenCalcDist_PS(IF_IS_PIXEL
                           Vec2 inTex:TEXCOORD):COLOR
{
	Vec4 dist=Vec4(0,0,0,0);
	Vec2 edge=Tex(Col,inTex).xy;
	//if(edge.x==0 && edge.y==0)clip(-1);
#if MODEL!=SM_2
	BRANCH if(edge.y)
#endif
	{
		// scan left
		{
		   Vec2 tex=inTex; tex.y+=0.5f*PixSize.y; // use bilinear filtering to scan both top and bottom
		   Flt  inc=1.0f/255;
		   UNROLL for(Int i=0; i<4; i++)
		   {
            tex.x-=PixSize.x; Vec2 edge=TexLod(Col,tex).xy;
            if( edge.r)inc=0; // top or bottom edge found
            if(!edge.g)inc=0;
            dist.x+=inc;
		   }
		}
		// scan right
		{
		   Vec2 tex =inTex; tex.y+=0.5f*PixSize.y; // use bilinear filtering to scan both top and bottom
		   Flt  inc =1.0f/255;
		   Vec2 edge=TexLod(Col,tex).xy;
		   UNROLL for(Int i=0; i<3; i++)
		   {
            if( edge.r)inc=0; // top or bottom edge found
            tex.x+=PixSize.x; edge=TexLod(Col,tex).xy;
            if(!edge.g)inc=0;			
            dist.y+=inc;
		   }
		}
	}
#if MODEL!=SM_2
	BRANCH if(edge.x)
#endif
	{
		// scan top
		{
		   Vec2 tex=inTex; tex.x+=0.5f*PixSize.x; // use bilinear filtering to scan both left and right
		   Flt  inc=1.0f/255;
		   UNROLL for(Int i=0; i<4; i++)
		   {
            tex.y-=PixSize.y; Vec2 edge=TexLod(Col,tex).xy;
            if( edge.g)inc=0; // horizontal edge on right or left
            if(!edge.r)inc=0;			
            dist.z+=inc;
		   }
		}
		// scan bottom
		{
         Vec2 tex =inTex; tex.x+=0.5f*PixSize.x; // use bilinear filtering to scan both left and right
         Flt  inc =1.0f/255;		
         Vec2 edge=TexLod(Col,tex).xy;
         UNROLL for(Int i=0; i<3; i++)
         {
            if( edge.g)inc=0; // horizontal edge on right or left			
            tex.y+=PixSize.y; edge=TexLod(Col,tex).xy;
            if(!edge.r)inc=0;			
            dist.w+=inc;
		   }
		}
	}
	return dist;
}
/*Vec4 EdgeSoftenUpdateDist1_PS(IF_IS_PIXEL
                                Vec2 inTex:TEXCOORD):COLOR
{
	Vec4   dist   =Tex(Col,inTex), offset=dist*(PixSize*255).xxyy;
	       dist.x+=Tex(Col,inTex-Vec2(offset.x,0)).x;
	       dist.y+=Tex(Col,inTex+Vec2(offset.y,0)).y;
	       dist.z+=Tex(Col,inTex-Vec2(0,offset.z)).z;
	       dist.w+=Tex(Col,inTex+Vec2(0,offset.w)).w;
	return dist;
}
Vec4 EdgeSoftenUpdateDist_PS(IF_IS_PIXEL
                             Vec2 inTex:TEXCOORD):COLOR
{
	Vec4 deltaPos  =Tex(Col,inTex);
	Vec4 result    =deltaPos;
	Vec2 texSize255=PixSize*255; deltaPos*=texSize255.xxyy;
	// LEFT
	BRANCH if(result.r>=MLAAThreshold2)
	{
		Vec2 currUV=inTex-Vec2(deltaPos.r,0);
		UNROLL for(Int i=0; i<3; i++)
		{
			Flt deltaU=TexLod(Col,currUV).r;
			result.r+=deltaU;
			currUV.x-=deltaU*texSize255.x;
		}
	}
	// RIGHT
	BRANCH if(result.g>=MLAAThreshold2)
	{
		Vec2 currUV=inTex+Vec2(deltaPos.g,0);
		UNROLL for(Int i=0; i<3; i++)
		{
			Flt deltaU=TexLod(Col,currUV).g;
			result.g+=deltaU;
			currUV.x+=deltaU*texSize255.x;
		}
	}
	// TOP
	BRANCH if(result.b>=MLAAThreshold2)
	{
		Vec2 currUV=inTex-Vec2(0,deltaPos.b);
		UNROLL for(Int i=0; i<3; i++)
		{
			Flt deltaV=TexLod(Col,currUV).b;
			result.b+=deltaV;
			currUV.y-=deltaV*texSize255.y;
		}
	}
	// BOTTOM
	BRANCH if(result.a>=MLAAThreshold2)
	{
		Vec2 currUV=inTex+Vec2(0,deltaPos.a);
		UNROLL for(Int i=0; i<3; i++)
		{
			Flt deltaV=TexLod(Col,currUV).a;
			result.a+=deltaV;
			currUV.y+=deltaV*texSize255.y;
		}
	}
	return result;
}*/


//#define SCAN_EDGES
//Col1=EdgeDetect
//Col2=Distance

/*// Scan horizontal edge
int scanHEdge(Vec2 uv, out Vec2 startUV, out Vec2 endUV, out int negLength, out int posLength)
{
	negLength=0;
	posLength=0;
	int length=1;
	startUV=endUV=uv;
	// scan left
	for(;;)
	{
		startUV.x -= PixSize.x;
		// current texel
		Vec2 edge=tex2Dlod(Col1,Vec4(startUV,0.f,0.f)).xy;
		// bottom texel
		Vec2 edgeB=tex2Dlod(Col1,Vec4(startUV+Vec2(0.f,PixSize.y),0.f,0.f)).xy;

		if(!edge.y || edge.x || edgeB.x) // either the horizontal edge stops, or we meet a vertical bar (above or below)
		{
			// yes ? stop 
			startUV.x += PixSize.x;
			break;
		}
		++ negLength;
		++ length;
	}
	// scan right
	// current texel
	Vec2 edge=tex2Dlod(Col1,Vec4(endUV,0.f,0.f)).xy;
	// bottom texel
	Vec2 edgeB=tex2Dlod(Col1,Vec4(endUV+Vec2(0.f,PixSize.y),0.f,0.f)).xy;	
	for(;;)
	{
		if(edge.x || edgeB.x) break; // met a vertical bar above or below ? -> stops
		endUV.x += PixSize.x;
		// current texel
		edge=tex2Dlod(Col1,Vec4(endUV,0.f,0.f)).xy;
		if(!edge.y) break; // test if horizontal edge stops
		// bottom texel
		edgeB=tex2Dlod(Col1,Vec4(endUV+Vec2(0.f,PixSize.y),0.f,0.f)).xy;	
		++ posLength;
		++ length;
	}
	return length;
} 

// Scan vertical edge
int scanVEdge(Vec2 uv, out Vec2 startUV, out Vec2 endUV, out int negLength, out int posLength)
{
	negLength=0;
	posLength=0;
	int length=1;
	startUV=endUV=uv;
	// scan up
	for(;;)
	{
		startUV.y -= PixSize.y;
		// current texel
		Vec2 edge=tex2Dlod(Col1,Vec4(startUV,0.f,0.f)).xy;
		// right texel
		Vec2 edgeR=tex2Dlod(Col1,Vec4(startUV+Vec2(PixSize.x,0.f),0.f,0.f)).xy;

		if(!edge.x || edge.y || edgeR.y) // either the vertical edge stops, or we met an horizontal bar (right or left)
		{
			// yes ? stop 
			startUV.y += PixSize.y;
			break;
		}
		++ negLength;
		++ length;
	}
	// scan bottom
	// current texel
	Vec2 edge=tex2Dlod(Col1,Vec4(endUV,0.f,0.f)).xy;
	// right texel
	Vec2 edgeR=tex2Dlod(Col1,Vec4(endUV+Vec2(PixSize.x,0.f),0.f,0.f)).xy;	
	for(;;)
	{
		if(edge.y || edgeR.y) break; // met a horizontal on the right or the left ? -> stops
		endUV.y += PixSize.y;
		// current texel
		edge=tex2Dlod(Col1,Vec4(endUV,0.f,0.f)).xy;
		if(!edge.x) break; // test if vertical edge stops
		// right texel
		edgeR=tex2Dlod(Col1,Vec4(endUV+Vec2(PixSize.x,0.f),0.f,0.f)).xy;	
		++ posLength;
		++ length;
	}
	return length;
}*/

inline Flt BlendFactor(Flt start, Flt end, Flt pos, Flt h0, Flt h1, Flt step)
{
	Flt slope=(h1 -h0)/(end+step-start),
	    hb0  = h0 +slope*(pos-start),
	    hb1  = hb0+step*slope,
	    area =(hb0+hb1)*0.5f;
	return area;
}

Vec4 EdgeSoftenApply_PS(IF_IS_PIXEL
                        Vec2 inTex:TEXCOORD):COLOR
{
	Vec2 edges =Tex(Col1,inTex                    ).xy,
	     edgesT=Tex(Col1,inTex+PixSize*Vec2( 0,-1)).xy,
	     edgesL=Tex(Col1,inTex+PixSize*Vec2(-1, 0)).xy;

   Vec4 finalCol=0;
   Flt  weight  =0;
	BRANCH if(edges.g+edges.r+edgesT.g+edgesL.r>0) // no edge found -> no need for anti-aliasing
   {
   #ifndef SCAN_EDGES
      Vec4 dist =TexLod(Col2,inTex                    ),
           distT=TexLod(Col2,inTex+PixSize*Vec2( 0,-1)),
           distL=TexLod(Col2,inTex+PixSize*Vec2(-1, 0));	
   #endif

      // BOTTOM HORIZONTAL EDGE
      if(edges.g)
      {
	      Flt  negLength, posLength, length;
	      Vec2 startUV, endUV;
      #ifdef SCAN_EDGES
	      length=scanHEdge(inTex, startUV, endUV, negLength, posLength);
      #else
	      // Use distance from the distance texture
	      negLength=dist.r*255;
	      posLength=dist.g*255;
	      length   =negLength+posLength+1;
	      startUV  =inTex+Vec2(-PixSize.x*negLength,0);
	      endUV    =inTex+Vec2( PixSize.x*posLength,0);
      #endif		

	      // test |_____ case
	      if(negLength<posLength) // if(length>1 && negLength<=posLength) // passed the middle, the contribution is NULL
	      {
		      Vec4 c=TexLod(Col1,startUV+PixSize*Vec2(-1,0));
		      if(c.r)
		      {
			      Flt h0= 0.5f,
			          h1=-0.5f,
			          b = BlendFactor(startUV.x, endUV.x, inTex.x, h0, h1, PixSize.x);				
			      finalCol+=TexLod(Col,inTex+Vec2(0,b*PixSize.y)); // lerp, using bilinear filtering for transition
			      weight  +=1;
		      }
	      }else

	      // test _____| case
	      if(negLength>posLength) // if(length>1 && negLength>=posLength) // passed the middle, the contribution is NULL
	      {			
		      Vec4 c=TexLod(Col1,endUV);	
		      if(c.r)
		      {
			      Flt h0=-0.5f,
			          h1= 0.5f,
			          b = BlendFactor(startUV.x, endUV.x, inTex.x, h0, h1, PixSize.x);				
			      finalCol+=TexLod(Col,inTex+Vec2(0,b*PixSize.y)); // lerp, using bilinear filtering for transition
			      weight  +=1;
		      }
	      }				
      }else

      // TOP HORIZONTAL EDGE
      if(edgesT.g)
      {		
	      Flt  negLength, posLength, length;
	      Vec2 startUV, endUV;
      #ifdef SCAN_EDGES
	      length=scanHEdge(inTex+PixSize*Vec2(0,-1), startUV, endUV, negLength, posLength);
      #else
	      // use distance from the distance texture
	      negLength=distT.r*255;
	      posLength=distT.g*255;
	      length   =negLength+posLength+1;
	      startUV  =inTex+Vec2(-PixSize.x*negLength,-PixSize.y);
	      endUV    =inTex+Vec2( PixSize.x*posLength,-PixSize.y);
      #endif	

	      //      ______ 
	      // test |       case
	      if(negLength<posLength) // if(length>1 && negLength<=posLength) // passed the middle, the contribution is NULL
	      {
		      Vec4 c=TexLod(Col1,startUV+PixSize*Vec2(-1,1));
		      if(c.r)
		      {
			      Flt h0=-0.5f,
			          h1= 0.5f,
			          b = BlendFactor(startUV.x, endUV.x, inTex.x, h0, h1, PixSize.x);				
			      finalCol+=TexLod(Col,inTex+Vec2(0,b*PixSize.y)); // lerp, using bilinear filtering for transition
			      weight  +=1;
		      }
	      }else

	      //      ______
	      // test      | case
	      if(negLength>posLength) // if(length>1 && negLength>=posLength) // passed the middle, the contribution is NULL
	      {
		      Vec4 c=TexLod(Col1,endUV+PixSize*Vec2(0,1));
		      if(c.r)
		      {
			      Flt h0= 0.5f,
			          h1=-0.5f,
			          b = BlendFactor(startUV.x, endUV.x, inTex.x, h0, h1, PixSize.x);				
			      finalCol+=TexLod(Col,inTex+Vec2(0,b*PixSize.y)); // lerp, using bilinear filtering for transition
			      weight  +=1;
		      }
	      }
      }

      // VERTICAL EDGE ON THE RIGHT
      if(edges.r)
      {
	      Flt  negLength, posLength, length;
	      Vec2 startUV, endUV;
      #ifdef SCAN_EDGES
	      length=scanVEdge(inTex, startUV, endUV, negLength, posLength);
      #else
	      // use distance from the distance texture
	      negLength=dist.b*255;
	      posLength=dist.a*255;
	      length   =negLength+posLength+1;
	      startUV  =inTex+Vec2(0,-PixSize.y*negLength);
	      endUV    =inTex+Vec2(0, PixSize.y*posLength);
      #endif		

	      // test _ case
	      //       |
	      //       |
	      if(negLength<posLength) // if(length>1 && negLength<=posLength) // passed the middle, the contribution is NULL
	      {
		      Vec4 c=TexLod(Col1,startUV+PixSize*Vec2(0,-1));
		      if(c.g)
		      {
			      Flt h0= 0.5f,
			          h1=-0.5f,
			          b = BlendFactor(startUV.y, endUV.y, inTex.y, h0, h1, PixSize.y);				
			      finalCol+=TexLod(Col,inTex+Vec2(b*PixSize.x,0)); // lerp, using bilinear filtering for transition
			      weight  +=1;
		      }
	      }else

	      // test  | case
	      //       |
	      //      --
	      if(negLength>posLength) // if(length>1 && negLength>=posLength) // passed the middle, the contribution is NULL
	      {			
		      Vec4 c=TexLod(Col1,endUV);
		      if(c.g)
		      {
			      Flt h0=-0.5f,
			          h1= 0.5f,				
			          b = BlendFactor(startUV.y, endUV.y, inTex.y, h0, h1, PixSize.y);				
			      finalCol+=TexLod(Col,inTex+Vec2(b*PixSize.x,0)); // lerp, using bilinear filtering for transition
			      weight  +=1;
		      }
	      }
      }else

      // VERTICAL EDGE ON THE LEFT
      if(edgesL.r)
      {		
	      Flt  negLength, posLength, length;
	      Vec2 startUV, endUV;
      #ifdef SCAN_EDGES
	      length=scanVEdge(inTex+PixSize*Vec2(-1,0), startUV, endUV, negLength, posLength);
      #else
	      // use distance from the distance texture
	      negLength=distL.b*255;
	      posLength=distL.a*255;
	      length   =negLength+posLength+1;
	      startUV  =inTex+Vec2(-PixSize.x,-PixSize.y*negLength);
	      endUV    =inTex+Vec2(-PixSize.x, PixSize.y*posLength);
      #endif			

	      // test _ case
	      //     |
	      //     |
	      if(negLength<posLength) // if(length>1 && negLength<=posLength) // passed the middle, the contribution is NULL
	      {
            Vec4 c=TexLod(Col1,startUV+PixSize*Vec2(1,-1));
            if(c.g)
            {
               Flt h0=-0.5f,
                   h1= 0.5f,
                   b = BlendFactor(startUV.y, endUV.y, inTex.y, h0, h1, PixSize.y);				
               finalCol+=TexLod(Col,inTex+Vec2(b*PixSize.x,0)); // lerp, using bilinear filtering for transition
               weight  +=1;
            }
	      }else

	      // test | case
	      //      |
	      //      --
	      if(negLength>posLength) // if(length>1 && negLength>=posLength) // passed the middle, the contribution is NULL
	      {
		      Vec4 c=TexLod(Col1,endUV+PixSize*Vec2(1,0));
		      if(c.g)
		      {
			      Flt h0= 0.5f,
			          h1=-0.5f,
			          b = BlendFactor(startUV.y, endUV.y, inTex.y, h0, h1, PixSize.y);				
			      finalCol+=TexLod(Col,inTex+Vec2(b*PixSize.x,0)); // lerp, using bilinear filtering for transition
			      weight  +=1;
		      }
	      }
      }
   }

   finalCol+=(1.0f/256)*Tex(Col,inTex);
   weight  +=(1.0f/256);
   return finalCol/weight;
}
/******************************************************************************/
#if MODEL!=SM_2
  TECHNIQUE(EdgeSoftenEdgeDetect , Draw_VS(), EdgeSoftenEdgeDetect_PS ());
  TECHNIQUE(EdgeSoftenCalcDist   , Draw_VS(), EdgeSoftenCalcDist_PS   ());
//TECHNIQUE(EdgeSoftenUpdateDist1, Draw_VS(), EdgeSoftenUpdateDist1_PS());
//TECHNIQUE(EdgeSoftenUpdateDist , Draw_VS(), EdgeSoftenUpdateDist_PS ());
  TECHNIQUE(EdgeSoftenApply      , Draw_VS(), EdgeSoftenApply_PS      ());
#endif
/******************************************************************************/
