# rhd

This is a repository for common/useful C header file implementations I've made and use myself.
Feel free to use them in your own project.

# hash_map.h example

```c
struct hash_map *hm = hash_map_new(int);
for(int i = 0; i < 100000; ++i)
{
  char istr[128];
  snprintf(istr,sizeof(istr),"%d",i);
  hash_map_insert(hm, istr, (int){i});
}
hash_map_insert(hm, "test", (int){432432});

//hash_map_dump(hm);

void *p = hash_map_find(hm, "9382");
printf("p = %p\n", p);

hash_map_free(&hm);
```
