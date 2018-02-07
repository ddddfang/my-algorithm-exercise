/* linux/include/linux/rbtree.h */

#ifndef	_LINUX_RBTREE_H
#define	_LINUX_RBTREE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

//#pragma pack(4)
struct rb_node
{
	unsigned long  rb_parent_color;	  //存储父节点的地址(low 2 bit 为 0) + 本节点的 r/b 信息(就放在 low 2 bit 中)
#define	RB_RED		0
#define	RB_BLACK	1
	struct rb_node *rb_right;
	struct rb_node *rb_left;
}__attribute__((aligned(sizeof(long)))); //4bytes对齐,因此节点地址的 low 2 bit 必然为 0

struct rb_root
{
	struct rb_node *rb_node;
};

#define rb_parent(r)   ((struct rb_node *)((r)->rb_parent_color & ~3))
#define rb_color(r)   ((r)->rb_parent_color & 1)

#define rb_is_red(r)   (!rb_color(r))
#define rb_is_black(r) rb_color(r)
#define rb_set_red(r)  do { (r)->rb_parent_color &= ~1; } while (0)
#define rb_set_black(r)  do { (r)->rb_parent_color |= 1; } while (0)


static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->rb_parent_color = (rb->rb_parent_color & 3) | (unsigned long)p;
}

static inline void rb_set_color(struct rb_node *rb, int color)
{
	rb->rb_parent_color = (rb->rb_parent_color & ~1) | color;
}

#define RB_ROOT	(struct rb_root){ NULL, }

//jensen style
#ifndef container_of
#define container_of(node, container, member) \
  (container *) (((char*) (node)) - ((unsigned long) &((container *)0)->member))
#endif

#define	rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root)  ((root)->rb_node == NULL)
#define RB_EMPTY_NODE(node)  (rb_parent(node) == node)
#define RB_CLEAR_NODE(node)  (rb_set_parent(node, node))

extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);


extern struct rb_node *rb_next(const struct rb_node *);
extern struct rb_node *rb_prev(const struct rb_node *);
extern struct rb_node *rb_first(const struct rb_root *);
extern struct rb_node *rb_last(const struct rb_root *);


extern void rb_replace_node(struct rb_node *victim, struct rb_node *newn, 
			    struct rb_root *root);


static inline void rb_link_node(struct rb_node * node, struct rb_node * parent,
				struct rb_node ** rb_link)
{
	node->rb_parent_color = (unsigned long )parent;  //parent地址low 2 bit为0,因此默认红色节点
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}
#ifdef __cplusplus
}
#endif

#endif	/* _LINUX_RBTREE_H */
