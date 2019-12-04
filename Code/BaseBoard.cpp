/// \file BaseBoard.cpp
/// \brief Code for the base chessboard CBaseBoard.

// MIT License
//
// Copyright (c) 2019 Ian Parberry
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "BaseBoard.h"

#include "Defines.h"
#include "Includes.h"
#include "Helpers.h"

extern MoveDeltas g_vecDeltas; ///< Move deltas for all possible knight's moves. 

/// Construct an empty board.

CBaseBoard::CBaseBoard(){
} //constructor

/// Construct a square undirected board.
/// \param n Board width and height.

CBaseBoard::CBaseBoard(unsigned n): CBaseBoard(n, n){
} //constructor

/// Construct a rectangular undirected board.
/// \param w Board width.
/// \param h Board height.

CBaseBoard::CBaseBoard(unsigned w, unsigned h):
  m_nWidth(w), m_nHeight(h), m_nSize(w*h)
{
  if(!(m_nSize & 1)){ //size must be even
    m_nMove = new int[m_nSize]; //create move table

    for(unsigned i=0; i<m_nSize; i++) //initialize move table
      m_nMove[i] = UNUSED;
  } //if
  
  m_cRandom.srand();
} //constructor

/// Construct an undirected board from a move table.
/// \param w Board width.
/// \param h Board height.
/// \param move A \f$w \times h\f$ move table.

CBaseBoard::CBaseBoard(int move[], unsigned w, unsigned h):
  m_nWidth(w), m_nHeight(h), m_nSize(w*h)
{
  if(!(m_nSize & 1)){ //size must be even
    m_nMove = new int[m_nSize]; //create move table

    for(unsigned i=0; i<m_nSize; i++) //copy move table
      m_nMove[i] = move[i];
  } //if
} //constructor

/// Delete the move tables.

CBaseBoard::~CBaseBoard(){
  delete [] m_nMove;
  delete [] m_nMove2;
} //destructor

/// Make every entry in the primary move table UNUSED and delete 
/// the secondary move table so that the cleared board is undirected.

void CBaseBoard::Clear(){
  for(unsigned i=0; i<m_nSize; i++)
    m_nMove[i] = UNUSED;

  delete [] m_nMove2;
  m_nMove2 = nullptr;
} //Clear

/// Test whether a cell index is in the correct range
/// to be on the board.
/// \param index Cell index to test.
/// \return true if cell index is actually on the board.

bool CBaseBoard::CellIndexInRange(int index){
  return 0 <= index && index < (int)m_nSize;
} //CellIndexInRange

/// Test whether a horizontal coordinate is on the board.
/// \param x An x-coordinate.
/// \return True if the x-coordinate is on the board.

bool CBaseBoard::InRangeX(int x){
  return 0 <= x && x < (int)m_nWidth;
} //InRangeX

/// Test whether a vertical coordinate is on the board.
/// \param y A y-coordinate.
/// \return True if the y-coordinate is on the board.

bool CBaseBoard::InRangeY(int y){
  return 0 <= y && y < (int)m_nHeight;
} //InRangeY

/// Test whether a move is recorded in the move tables.
/// \param i Board index.
/// \param j Board index.
/// \return true if there is a move from cell i to cell j on the board.

bool CBaseBoard::IsMove(int i, int j){
  return CellIndexInRange(i) && CellIndexInRange(j) &&
    (m_nMove[i] == j || m_nMove2[i] == j ||
     m_nMove[j] == i || m_nMove2[j] == i);
} //IsMove

/// Test whether two cells are separated by a knight's move.
/// \param i Board cell index.
/// \param j Board cell index.
/// \return true if j is a knight's move from i (and vice-versa)

bool CBaseBoard::IsKnightMove(int i, int j){
  const int n = (int)m_nSize;

  if(0 <= i && i < n && 0 <= j && j < n) //safety check 
    for(MoveDelta delta: g_vecDeltas)
      if(IsOnBoard(i, delta)){
        const int x = i%m_nWidth + delta.first;
        const int y = i/m_nWidth + delta.second;

        if(j == y*m_nWidth + x)
          return true;
      } //if

  return false;
} //IsKnightMove

/// Test whether a cell is unused. Cells outside of the board 
/// are reported to be used. Note that this is different from
/// the behaviour of GetMove(), which reports cells outside
/// the board to be unused. Assumes that the board is undirected.
/// \param index Cell index.
/// \return true if the cell with that index is unused.

bool CBaseBoard::IsUnused(int index){
  assert(IsDirected()); //safety
  return CellIndexInRange(index) && m_nMove[index] == UNUSED;
} //IsUnused

/// Test whether a move ends up in an occupied (used) cell in
/// a partially constructed knight's tour or tourney. If the
/// move takes us off the board, then the cell is reported as used.
/// Assumes that the board is undirected.
/// \param pos Board cell index.
/// \param d Move deltas for a knight's move.
/// \return true If the cell move d away from pos is unused.

bool CBaseBoard::IsUnused(int pos, const MoveDelta& d){
  assert(IsUndirected()); //safety
  if(!CellIndexInRange(pos))return false;
  
  const int x = pos%m_nWidth + d.first; //destination column
  const int y = pos/m_nWidth + d.second; //destination row

  return InRangeX(x) && InRangeY(y) && m_nMove[y*m_nWidth + x] == UNUSED;
} //IsUnused

/// Test whether a move stays on the board.
/// Assumes that the board is undirected.
/// \param pos Cell index.
/// \param d Move delta.
/// \return true If the cell move d away from pos is on the board.

bool CBaseBoard::IsOnBoard(int pos, const MoveDelta& d){
  assert(IsUndirected()); //safety
  if(!CellIndexInRange(pos))return false;
  
  const int x = pos%m_nWidth + d.first; //destination column
  const int y = pos/m_nWidth + d.second; //destination row

  return InRangeX(x) && InRangeY(y);
} //IsOnBoard

/// Count the number of available moves from a given cell,
/// that is, knight's moves that stay on the board and go
/// to an unused cell. Assumes that the board is undirected.
/// \param index Board cell index.
/// \return Number of moves from that that end up in unoccupied cells.

int CBaseBoard::GetAvailableMoveCount(int index){
  assert(IsUndirected()); //safety

  const int x0 = index%m_nWidth; //cell column
  const int y0 = index/m_nWidth; //cell row

  int count = 0; //return value

  for(MoveDelta delta: g_vecDeltas){
    const int x = x0 + delta.first; //destination column of move delta
    const int y = y0 + delta.second; //destination row of move delta
    const int dest = y*m_nWidth + x; //destination index of move delta

    if(InRangeX(x) && InRangeY(y) && m_nMove[dest] == UNUSED)
      count++;
  } //for

  return count;
} //GetAvailableMoveCount

/// Get move from a cell. Cells outside of the board are reported
/// as UNUSED. Note that this is different from the behaviour of
/// IsUnused(), which reports cells outside the board to be used.
/// Assumes that the board is undirected.
/// \param index Cell index.
/// \return Move from the cell with that index.

int CBaseBoard::operator[](int index){
  assert(IsUndirected()); //safety
  return CellIndexInRange(index)? m_nMove[index]: UNUSED;
} //operator[]

/// Knight's tour test for both directed and undirected boards.
/// \return true if this board contains a closed knight's tour.

bool CBaseBoard::IsTour(){
  int prev = 0;
  int cur = m_nMove[0];
  unsigned count = 1;

  //now we can begin the tour

  while(count < m_nSize && CellIndexInRange(cur) && cur != 0){
    const int dest0 = m_nMove[cur];
    const int newprev = cur;

    if(dest0 == prev){
      if(IsUndirected())return false; //this should never happen
      cur = m_nMove2[cur];
    } //if

    else cur = dest0;

    prev = newprev;
    count++;
  } //while

  return count == m_nSize && cur == 0;
} //IsTour

/// Tourney test for both directed and undirected boards.
/// \return true if this board contains a tourney.

bool CBaseBoard::IsTourney(){
  int *count = new int[m_nSize];

  for(unsigned i=0; i<m_nSize; i++)
    count[i] = 0;

  //all cells must be used

  bool bAllCellsUsed = true;

  if(IsUndirected()){ //board is undirected
    for(unsigned i=0; i<m_nSize && bAllCellsUsed; i++){
      if(CellIndexInRange(m_nMove[i])){
        ++count[i];
        ++count[m_nMove[i]];
      } //if
      else bAllCellsUsed = false;
    } //for
  } //if

  else{ //board is directed
    for(unsigned i=0; i<m_nSize && bAllCellsUsed; i++){
      if(CellIndexInRange(m_nMove[i]))
        ++count[m_nMove[i]];
      else bAllCellsUsed = false;

      if(CellIndexInRange(m_nMove2[i]))
        ++count[m_nMove2[i]];
      else bAllCellsUsed = false;
    } //for
  } //else

  if(!bAllCellsUsed)return false;

  //the degree of the graph must 2

  bool bDegree2 = true;

  if(bAllCellsUsed)
    for(unsigned i=0; i<m_nSize; i++)
      if(count[i] != 2)
        bDegree2 = false;

  delete [] count;

  return bDegree2;
} //IsTourney

/// Test whether the board is directed, that is, it is not undirected.
/// \return true If the board is directed.

bool CBaseBoard::IsDirected(){
  return !IsUndirected();
} //IsDirected

/// Test whether the board is undirected. If it is,
/// then m_nMove2 will be NULL and vice-versa.
/// \return true If the board is undirected.

bool CBaseBoard::IsUndirected(){
  return m_nMove2 == nullptr;
} //IsUndirected

/// Make into a directed board by creating the second
/// move table and copying the edges in the first move table
/// into back edges in the second one. 

void CBaseBoard::MakeDirected(){
  if(IsUndirected()){
    m_nMove2 = new int[m_nSize];
    for(unsigned i=0; i<m_nSize; i++)
      m_nMove2[i] = UNUSED;

    for(unsigned i=0; i<m_nSize; i++)
      if(CellIndexInRange(m_nMove[i]))
       m_nMove2[m_nMove[i]] = i;
  } //if
} //MakeDirected

/// Make into an undirected board by reorganizing the move order.
/// It is assumed that the directed board contains a tourney.
/// If not, then this function does nothing.

void CBaseBoard::MakeUndirected(){
  if(IsDirected() && IsTourney()){   
    int* temp = new int[m_nSize]; 
  
    for(unsigned i=0; i<m_nSize; i++)
      temp[i] = UNUSED;

    for(unsigned start=0; start<m_nSize; start++)
      if(temp[start] == UNUSED){

        int prev = start;
        int cur = m_nMove[start];

        while(CellIndexInRange(cur) && cur != start){ 
          temp[prev] = cur;   

          const int dest0 = (m_nMove[cur] == prev)? m_nMove2[cur]: m_nMove[cur];
          prev = cur;
          cur = dest0;
        } //while
        
        if(CellIndexInRange(prev) && CellIndexInRange(cur)){
          temp[prev] = cur;
        } //if
      } //if

    //clean up and exit
    
    delete [] m_nMove;
    m_nMove = temp;

    delete [] m_nMove2;
    m_nMove2 = nullptr;
  } //if
} //MakeUndirected

/// Compute the destination of a move, given the cell index and the
/// horizontal and vertical deltas of a move.
/// \param i Cell index.
/// \param delta Move deltas.
/// \return Destination of the move from cell i with move deltas delta.

int CBaseBoard::GetDest(int i, const MoveDelta& delta){
  int x = i%m_nWidth + delta.first;
  int y = i/m_nWidth + delta.second;

  if(InRangeX(x) && InRangeY(y)) //move stays on board
    return y*m_nWidth + x;
  else return UNUSED; //fail
} //GetDest

/// Compute the index of a knight's move given the indexes of the cells.
/// Returns UNUSED if the move is not as knight's move.
/// \param src Index of source cell.
/// \param dest Index of destination cell.
/// \return The move index of the knight's move from src to dest.

int CBaseBoard::GetMoveIndex(int src, int dest){
  const int dx = dest%m_nWidth - src%m_nWidth;
  const int dy = dest/m_nWidth - src/m_nWidth;

  switch(dx){
    case -2: 
      if(dy == -1)return 3;
      else if(dy == 1)return 4;
    break;

    case -1: 
      if(dy == -2)return 2;
      else if(dy == 2)return 5;
    break;

    case 1: 
      if(dy == -2)return 1;
      else if(dy == 2)return 6;
    break;
    
    case 2: 
      if(dy == -1)return 0;
      else if(dy == 1)return 7;
    break;
  } //switch

  return UNUSED; //bother, it's not a knight's move
} //GetMoveIndex

/// Copy a board into a sub-board of this board. 
/// Assumes that the board to be copied in is undirected.
/// \param b Undirected board to copy in.
/// \param x0 Column of first cell in which to copy b.
/// \param y0 Row of first cell in which to copy b.

void CBaseBoard::CopyToSubBoard(CBaseBoard& b, int x0, int y0){
  assert(b.IsUndirected()); //safety

  const int w = b.m_nWidth;
  const int h = b.m_nHeight;
  
  for(int bsrcy=0; bsrcy<h; bsrcy++)
    for(int bsrcx=0; bsrcx<w; bsrcx++){
      const int bsrc = bsrcy*w + bsrcx;
      const int bdest = b[bsrc];

      const int bdestx = bdest%w;
      const int bdesty = bdest/w;

      const int srcx = bsrcx + x0;
      const int srcy = bsrcy + y0;
      
      const int destx = bdestx + x0;
      const int desty = bdesty + y0;

      const int src = srcy*m_nWidth + srcx;
      const int dest = desty*m_nWidth + destx;

      if(IsDirected())
        InsertDirectedMove(src, dest);
      else InsertUndirectedMove(src, dest);
    } //for
} //CopyToSubBoard

////////////////////////////////////////////////////////////////////////
// Move insertion and deletion functions.

#pragma region Move insertion and deletion

/// Insert an undirected move. Assumes that the board is undirected.
/// \param src One end of move to insert.
/// \param dest The other end of move to insert.
/// \return true if the insert was successful (the cells were unused).

bool CBaseBoard::InsertUndirectedMove(int src, int dest){
  assert(IsUndirected()); //safety

  if(m_nMove[src] < 0)
    m_nMove[src] = dest;

  else if(m_nMove[dest] < 0)
    m_nMove[dest] = src;

  else return false;

  return true;
} //InsertMove

/// Insert a directed move. Assumes that the board is directed.
/// \param src One end of move to insert.
/// \param dest The other end of move to insert.
/// \return true if the insert was successful (the cells were unused).

bool CBaseBoard::InsertDirectedMove(int src, int dest){
  assert(IsDirected()); //safety

  if(m_nMove[src] < 0)
    m_nMove[src] = dest;

  else if(m_nMove2[src] < 0)
    m_nMove2[src] = dest;

  else return false;
  
  if(m_nMove[dest] < 0)
    m_nMove[dest] = src;

  else if(m_nMove2[dest] < 0)
    m_nMove2[dest] = src;

  else return false;

  return true;
} //InsertDirectedMove

/// Delete a move. Works for both directed and undirected boards.
/// \param src One end of move to delete.
/// \param dest The other end of move to delete.
/// \return true if the delete was successful (the move was there).

bool CBaseBoard::DeleteMove(int src, int dest){ 
  //delete move from primary move table

  if(m_nMove[src] == dest) 
    m_nMove[src] = UNUSED;

  if(m_nMove[dest] == src) 
    m_nMove[dest] = UNUSED;

  //delete move from secondary move table

  if(IsDirected()){  
    if(!IsMove(src, dest))
      return false;

    if(m_nMove2[src] == dest) 
      m_nMove2[src] = UNUSED;

    if(m_nMove2[dest] == src) 
      m_nMove2[dest] = UNUSED;
  } //if

  return true;
} //DeleteMove

#pragma endregion Move insertion and deletion

////////////////////////////////////////////////////////////////////////
// Save functions.

#pragma region Save functions

/// Save the board's move table to a text file. For example, if you saved an
/// \f$8 \times 8\f$ knight's tour, then it might look like the image below. 
/// For each board cell from top to bottom and left to right, the file lists
/// the index of the move (in row-major order) that a knight on the knight's
/// tour or tourney will use next when in cell. Assumes that the board is
/// undirected.
///
/// \image html Textfile.png
///
/// \param name Root of file name.

void CBaseBoard::Save(std::string& name){
  assert(IsUndirected()); //safety

  char buffer[256];
  sprintf_s(buffer, "%s.txt", name.c_str());

  FILE* output = nullptr;
  fopen_s(&output, buffer, "wt");

  if(output != nullptr){
    for(unsigned i=0; i<m_nHeight; i++){
      for(unsigned j=0; j<m_nWidth; j++){
        const int cell = i*m_nWidth + j;
        fprintf_s(output, "%d", GetMoveIndex(cell, m_nMove[cell]));
      } //for
      fprintf_s(output, "\n");
    } //for

    fclose(output);
  } //if
} //Save

/// Save the board to an SVG file suitable for display on a web page. Open
/// it up in your favorite browser. SVG files are very large, but they compress
/// very well as a zip file. The upside is that they are vector graphics, so
/// they zoom very well without pixelization.
/// Assumes that the board is undirected.
/// \param name Root of file name.

void CBaseBoard::SaveToSVG(std::string& name){  
  assert(IsUndirected()); //safety

  char buffer[256];
  sprintf_s(buffer, "%s.svg", name.c_str());
  
  FILE* output = nullptr;
  fopen_s(&output, buffer, "wt");
  
  if(output != nullptr){
    const int w = m_nWidth; //shorthand
    const int h = m_nHeight; //shorthand
    const int n = m_nSize; //shorthand

    float scale = 0.5f;
    if(m_nSize > 100000)scale = 0.01f;
    else if(m_nSize > 16384)scale = 0.125f;
    else if(m_nSize > 1024)scale = 0.25f;
  
    const float cellsize0 = 32; //cell size before scaling
    const float cellsize = scale*cellsize0; //scale the cell size
    const float spotsize = cellsize/6.0f; //spot size
    const float strokewidth = 2.0f*scale;

    fprintf(output, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");

    const unsigned sw = (unsigned)std::ceil(w*cellsize); //screen width
    const unsigned sh = (unsigned)std::ceil(h*cellsize); //screen height

    fprintf(output, "<svg width=\"%u\" height=\"%u\" ", sw + 8, sh + 8);
    fprintf(output, "viewBox=\"-4 -4 %u %u\" ", sw + 8, sh + 8);
    fprintf(output, "xmlns=\"http://www.w3.org/2000/svg\">");
    fprintf(output, "<style>");
    fprintf(output, "circle{fill:black;r:%0.1f}", spotsize);	
    fprintf(output, "line{stroke:black;stroke-width:%0.1f}", 2.0f*strokewidth);
    fprintf(output, "</style>");

    //board

    fprintf(output, "<rect width=\"%u\" height=\"%u\" ", sw, sh);
    fprintf(output, "style=\"fill:white;stroke:black;stroke-width:%s\"/>",
      NumString(strokewidth).c_str());

    const std::string bds = "style=\"stroke-width:" + NumString(strokewidth) + "\"";
    
    for(int i=1; i<h; i++)
      fprintf(output, "<line x1=\"0\" y1=\"%s\" x2=\"%s\" y2=\"%s\" %s/>",
        NumString((float)i*cellsize).c_str(),
        NumString((float)w*cellsize).c_str(), 
        NumString((float)i*cellsize).c_str(), 
        bds.c_str()
      );
    
    for(int i=1; i<w; i++)
      fprintf(output, "<line x1=\"%s\" y1=\"0\" x2=\"%s\" y2=\"%s\" %s/>",
        NumString((float)i*cellsize).c_str(), 
        NumString((float)i*cellsize).c_str(), 
        NumString((float)h*cellsize).c_str(), 
        bds.c_str()
      );

    fprintf(output, "");

    //knight's tour

    for(int i=0; i<w; i++)
      for(int j=0; j<h; j++){
        const int src = j*w + i;
        const int dest = m_nMove[src];
  
        if(0 <= dest && dest < n){
          const float srcx = (i + 0.5f)*cellsize;
          const float srcy = (h - 1 - j + 0.5f)*cellsize;
  
          const float destx = (dest%w + 0.5f)*cellsize;
          const float desty = (h - 1 - std::floor((float)dest/w) + 0.5f)*cellsize;

          fprintf(output, "<line x1=\"%s\" y1=\"%s\" x2=\"%s\" y2=\"%s\"/>",
            NumString(srcx).c_str(),  NumString(srcy).c_str(),
            NumString(destx).c_str(), NumString(desty).c_str()
          );

          fprintf(output, "<circle cx=\"%s\" cy=\"%s\"/>",
            NumString(srcx).c_str(), NumString(srcy).c_str());
        } //if
      } //for

    fprintf(output, "</svg>\n");
    fclose(output);
  } //if
} //SaveToSVG

#pragma endregion Save functions

///////////////////////////////////////////////////////////////////////////
// Reader functions.

/// Reader function for width.
/// \return Width.

int CBaseBoard::GetWidth(){
  return m_nWidth;
} //GetWidth

/// Reader function for height.
/// \return Height.

int CBaseBoard::GetHeight(){
  return m_nHeight;
} //GetHeight

/// Reader function for size (width times height).
/// \return Size.

int CBaseBoard::GetSize(){
  return m_nSize;
} //GetSize
