import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaoElementComponent } from './nao-element.component';

describe('NaoElementComponent', () => {
  let component: NaoElementComponent;
  let fixture: ComponentFixture<NaoElementComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaoElementComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaoElementComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
