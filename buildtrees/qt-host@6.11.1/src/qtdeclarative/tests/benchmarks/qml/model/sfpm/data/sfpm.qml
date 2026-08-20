import QtQml

QtObject {
   id: sfpm

   // Filter
   property ValueFilter valueFilter: ValueFilter {}

   // Sorters
   property RoleSorter roleSorter: RoleSorter {}
   property StringSorter stringSorter: StringSorter {}

   property SortFilterProxyModel sfpmModel: SortFilterProxyModel {}
}
