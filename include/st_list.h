#ifndef _ST_LIST_H_
#define _ST_LIST_H_

#include <st_types.h>

struct st_list_head {
	struct st_list_head *next, *prev;
};

#define ST_LIST_HEAD_INIT(name)		{ &(name), &(name) }
#define ST_LIST_HEAD(name)			struct st_list_head name = ST_LIST_HEAD_INIT(name)

static inline void ST_LIST_INIT_HEAD(struct st_list_head *list) {
	list->next = list;
	list->prev = list;
}

static inline void __st_list_add(struct st_list_head *new_item, struct st_list_head *prev, struct st_list_head *next) {
    next->prev = new_item;
    new_item->next = next;
    new_item->prev = prev;
    prev->next = new_item;
}

static inline void st_list_add(struct st_list_head *new_item, struct st_list_head *head) {
    __st_list_add(new_item, head, head->next);
}

static inline void st_list_add_tail(struct st_list_head *new_item, struct st_list_head *head) {
    __st_list_add(new_item, head->prev, head);
}

static inline void __st_list_del(struct st_list_head *prev, struct st_list_head *next) {
	next->prev = prev;
	prev->next = next;
}

static inline void st_list_del(struct st_list_head *entry) {
	__st_list_del(entry->prev, entry->next);
}

static inline void __st_list_del_entry(struct st_list_head *entry) {
	__st_list_del(entry->prev, entry->next);
}

static inline void st_list_del_init(struct st_list_head *entry){
	__st_list_del_entry(entry);
	ST_LIST_INIT_HEAD(entry);
}

static inline int st_list_is_last(const struct st_list_head *list, const struct st_list_head *head) {
	return list->next == head;
}

static inline int st_list_empty(const struct st_list_head *head) {
	return head->next == head;
}

static inline void __st_list_splice(const struct st_list_head *list,
				 struct st_list_head *prev,
				 struct st_list_head *next) {
	struct st_list_head *first = list->next;
	struct st_list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

static inline void st_list_splice(const struct st_list_head *list,
				struct st_list_head *head) {
	if (!st_list_empty(list))
		__st_list_splice(list, head, head->next);
}

static inline void st_list_splice_tail_init(struct st_list_head *list,
					 struct st_list_head *head) {
	if (!st_list_empty(list)) {
		__st_list_splice(list, head->prev, head);
		ST_LIST_INIT_HEAD(list);
	}
}


#define st_list_entry(ptr, type, member)								\
	st_container_of(ptr, type, member)

#define st_list_first_entry(ptr, type, member)		 					\
	list_entry((ptr)->next, type, member)

#define st_list_for_each(pos, head) \
	for(pos = (head)->next; pos != (head); pos = pos->next)

#define st_list_for_each_entry(pos, head, member)						\
	for(pos = st_list_entry((head)->next, typeof(*pos), member);		\
		&pos->member != (head);											\
		pos = st_list_entry(pos->member.next, typeof(*pos), member))

#define st_list_for_each_entry_safe(pos, n, head, member)				\
	for (pos = st_list_entry((head)->next, typeof(*pos), member),		\
		n = st_list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head); 										\
	     pos = n, n = st_list_entry(n->member.next, typeof(*n), member))

#endif
