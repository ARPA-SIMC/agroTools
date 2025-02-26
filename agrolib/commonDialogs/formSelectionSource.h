#ifndef FORMSELECTIONSOURCE_H
#define FORMSELECTIONSOURCE_H

    #include <QtWidgets>

    class FormSelectionSource : public QDialog
    {
        Q_OBJECT

    public:
        FormSelectionSource(bool pointVisible, bool gridVisible, bool interpolationVisible);

        int getSourceSelectionId();

    private:
        QRadioButton* pointButton;
        QRadioButton* gridButton;
        QRadioButton* interpolationButton;

        void done(int res);
    };

#endif // FORMSELECTIONSOURCE_H
