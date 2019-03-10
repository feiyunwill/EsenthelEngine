/******************************************************************************/
template<typename TYPE> Game::ObjMap<TYPE>& GetObjContainer(Memx<Game::ObjMap<TYPE> > &containers, int &counter)
{
   Game::ObjMap<TYPE> &container=(InRange(counter, containers) ? containers[counter] : containers.New());
   counter++; return container;
}
/******************************************************************************/
