## kref
krefs allow you to add reference counters to your objects. If you have objects that are used in multiple places and passed around, and
you don't have refcounts, your code is almost certainly broken. If you want refcounts, krefs are the way to go.

To use a kref, add one to your data structures like:
```
struct my_data
{
	.
	.
	struct kref refcount;
	.
	.
};
```
The `kref` can occur anywhere within the data structure.

You must initialize the kref after you allocate it. To do this, call `kref_init` as so:
```
     struct my_data *data;

     data = kmalloc(sizeof(*data), GFP_KERNEL);
     if (!data)
            return -ENOMEM;
     kref_init(&data->refcount);
```
This sets the `refcount` in the `kref` to 1.

```
kref_get - increment refcount for object.
kref_put - decrement refcount for object, and if 0, call release().
```
Once you have an initialized `kref`, you must follow the following rules:

1) If the code attempts to gain a reference to a kref-ed structure without already holding a valid pointer, it must serialize access where a `kref_put()` cannot occur during the `kref_get()`, and the structure must remain valid during the `kref_get()`.

Say, for instance, you have a list of items that are each kref-ed, and you wish to get the first one. You can't just pull the first item off the list and `kref_get()` it. That violates rule 1 because you are not already holding a valid pointer. You must add a mutex (or some other lock). For instance:
```
static DEFINE_MUTEX(mutex);
static LIST_HEAD(q);
struct my_data
{
	struct kref      refcount;
	struct list_head link;
};

static struct my_data *get_entry()
{
	struct my_data *entry = NULL;
	mutex_lock(&mutex);
	if (!list_empty(&q)) {
		entry = container_of(q.next, struct my_data, link);
		kref_get(&entry->refcount);
	}
	mutex_unlock(&mutex);
	return entry;
}

static void release_entry(struct kref *ref)
{
	struct my_data *entry = container_of(ref, struct my_data, refcount);

	list_del(&entry->link);
	kfree(entry);
}

static void put_entry(struct my_data *entry)
{
	mutex_lock(&mutex);
	kref_put(&entry->refcount, release_entry);
	mutex_unlock(&mutex);
}
```

## container_of
```
container_of(ptr, type, member);
```
When given a pointer to a field within a structure-object, and the type-name for that that structure-object, and the field-name for that structure's field-member, then it returns the structure's address.

```
struct mystruct	{
		char		w;
		short		x;
		long		y;
		long long	z;
		} my_instance = { 1, 2, 3, 4 };	
```
If you have a pointer to one of the fields in some instance of a this kind of 'struct' object, then you could use the `container_of()` macro to get a pointer to that 'struct' object itself, like this:
```
     long		  *ptr = &my_instance.y;
     struct mystruct  *p = container_of( ptr, struct mystruct, y ); 
```